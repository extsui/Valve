﻿using OxyPlot;
using OxyPlot.Axes;
using OxyPlot.Series;
using Reactive.Bindings;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO.Ports;
using System.Windows;
using ValveDemo.Models;

namespace ValveDemo
{
    class ViewModel : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        public ReactiveCollection<string> ComPorts { get; set; } = new ReactiveCollection<string>();
        public ReactiveProperty<string> SelectedPort { get; set; } = new ReactiveProperty<string>();
        public ReactiveCollection<string> RxData { get; set; } = new ReactiveCollection<string>();
        public ReactiveProperty<string> LogData { get; set; } = new ReactiveProperty<string>();
        public ReactiveProperty<bool> ButtonConnect_IsEnabled { get; set; } = new ReactiveProperty<bool>(true);
        public ReactiveProperty<bool> ButtonDisconnect_IsEnabled { get; set; } = new ReactiveProperty<bool>(false);

        // スライダーと連動
        public ReactiveProperty<double> EncoderValue1 { get; set; } = new ReactiveProperty<double>(0);
        public ReactiveProperty<double> EncoderValue2 { get; set; } = new ReactiveProperty<double>(0);
        public ReactiveProperty<double> EncoderValue3 { get; set; } = new ReactiveProperty<double>(0);
        public ReactiveProperty<double> EncoderValue4 { get; set; } = new ReactiveProperty<double>(0);

        // グラフの描画と連動
        public ReactiveProperty<bool> EncoderEnabled1 { get; set; } = new ReactiveProperty<bool>(true);
        public ReactiveProperty<bool> EncoderEnabled2 { get; set; } = new ReactiveProperty<bool>(true);
        public ReactiveProperty<bool> EncoderEnabled3 { get; set; } = new ReactiveProperty<bool>(true);
        public ReactiveProperty<bool> EncoderEnabled4 { get; set; } = new ReactiveProperty<bool>(true);

        private SerialPortManager m_SerialPortManager;
        private Valve m_Valve;
        private ValveQueue m_ValveQueue;

        // OxyPlot 用
        public PlotController Controller { get; private set; } = new PlotController();
        private PlotModel _graphModel;
        public PlotModel GraphModel
        {
            get => _graphModel;
            set
            {
                _graphModel = value;
                PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(null));
            }
        }

        // グラフの設定
        public void InitGraph()
        {
            var plotModel = new PlotModel();

            // 軸の設定
            {
                var axisX = new LinearAxis();
                var axisY = new LinearAxis();

                // X軸の設定
                axisX.Position = AxisPosition.Bottom;    // 軸の位置(topにしたら、目盛りが上にくる)
                axisX.Minimum = 0;
                axisX.Maximum = 5;

                // Y軸の設定
                axisY.Position = AxisPosition.Left;      // Y軸の位置(Rightにしたら、目盛りが右にくる)
                axisY.Minimum = sbyte.MinValue;
                axisY.Maximum = sbyte.MaxValue;

                // 設定した軸をモデルにセット
                plotModel.Axes.Add(axisX);
                plotModel.Axes.Add(axisY);
            }

            GraphModel = plotModel;
        }

        public ViewModel()
        {
            m_SerialPortManager = new SerialPortManager();
            m_Valve = new Valve();
            m_ValveQueue = new ValveQueue();

            InitGraph();

            WriteLine("Valve Initialize.");
        }

        public void GetComPorts()
        {
            ComPorts.Clear();
            var ports = SerialPort.GetPortNames();
            foreach (var port in ports)
            {
                ComPorts.Add(port);
            }
        }

        public void SerialOpen()
        {
            try
            {
                m_SerialPortManager.Open(SelectedPort.Value);
                m_SerialPortManager.SetReceivedHandler(OnReceived);
                ButtonConnect_IsEnabled.Value = false;
                ButtonDisconnect_IsEnabled.Value = true;
            }
            catch (Exception e)
            {
                MessageBox.Show(e.Message);
            }
        }

        public void SerialClose()
        {
            try
            {
                m_SerialPortManager.Close();
                ButtonConnect_IsEnabled.Value = true;
                ButtonDisconnect_IsEnabled.Value = false;
            }
            catch (Exception e)
            {
                MessageBox.Show(e.Message);
            }
        }

        private void OnReceived(object sender, SerialDataReceivedEventArgs e)
        {
            string value = m_SerialPortManager.Read();

            // UI スレッド以外からの操作のため Add() では NG
            RxData.AddOnScheduler(value);

            var result = m_Valve.Update(value);
            if (!result)
            {
                WriteLine($"[warn] valve string error! (\"{value}\")");
            }

            m_ValveQueue.Enqueue(new Valve(m_Valve));
            
            // UI の更新
            EncoderValue1.Value = (double)m_Valve.EncoderValue[0];
            EncoderValue2.Value = (double)m_Valve.EncoderValue[1];
            EncoderValue3.Value = (double)m_Valve.EncoderValue[2];
            EncoderValue4.Value = (double)m_Valve.EncoderValue[3];

            // グラフの更新
            {
                var plotModel = new PlotModel();
                var queue = m_ValveQueue.ReadAll().ToArray();

                // 線グラフの設定
                {
                    var points1 = new List<DataPoint>();
                    var points2 = new List<DataPoint>();
                    var points3 = new List<DataPoint>();
                    var points4 = new List<DataPoint>();

                    // タイムスタンプ (ms) を元に秒単位の時間軸にする
                    foreach (var item in queue)
                    {
                        var x = (double)item.TimeStamp / 1000;
                        points1.Add(new DataPoint(x, item.EncoderValue[0]));
                        points2.Add(new DataPoint(x, item.EncoderValue[1]));
                        points3.Add(new DataPoint(x, item.EncoderValue[2]));
                        points4.Add(new DataPoint(x, item.EncoderValue[3]));
                    }

                    var colors = new List<OxyColor>()
                    {
                        OxyColor.FromArgb(0xFF, 246, 173, 60 ), // 橙
                        OxyColor.FromArgb(0xFF, 232, 82,  152), // 桃
                        OxyColor.FromArgb(0xFF, 0,   169, 95 ), // 緑
                        OxyColor.FromArgb(0xFF, 24,  127, 196), // 青
                    };

                    var encoderDatas = new List<List<DataPoint>>()
                    {
                        points1, points2, points3, points4,
                    };

                    var enabled = new List<bool>()
                    {
                        EncoderEnabled1.Value,
                        EncoderEnabled2.Value,
                        EncoderEnabled3.Value,
                        EncoderEnabled4.Value,
                    };

                    for (int i = 0; i < 4; i++)
                    {
                        if (!enabled[i])
                        {
                            continue;
                        }
                        var lineSeries = new LineSeries();
                        lineSeries.Title = $"#{i + 1}";
                        lineSeries.StrokeThickness = 10;
                        lineSeries.Color = colors[i];
                        lineSeries.ItemsSource = encoderDatas[i];

                        // グラフをモデルに追加
                        plotModel.Series.Add(lineSeries);
                    }
                }

                // 軸の設定
                {
                    var axisX = new LinearAxis();
                    var axisY = new LinearAxis();

                    // X軸の設定
                    axisX.Position = AxisPosition.Bottom;    // 軸の位置(topにしたら、目盛りが上にくる)
                    axisX.Minimum = (double)queue[0].TimeStamp / 1000;
                    axisX.Maximum = axisX.Minimum + 5.0;
                    axisX.Title = "時刻 (単位: 秒)";

                    // Y軸の設定
                    axisY.Position = AxisPosition.Left;      // Y軸の位置(Rightにしたら、目盛りが右にくる)
                    axisY.Minimum = sbyte.MinValue;
                    axisY.Maximum = sbyte.MaxValue;
                    axisY.Title = "エンコーダ値";

                    // 設定した軸をモデルにセット
                    plotModel.Axes.Add(axisX);
                    plotModel.Axes.Add(axisY);
                }

                GraphModel = plotModel;
                GraphModel.InvalidatePlot(true);
            }
        }

        public void WriteLine(string value)
        {
            LogData.Value += value + "\r\n";
        }
    }
}
