using OxyPlot;
using Reactive.Bindings;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
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

        public ReactiveProperty<double> EncoderValue1 { get; set; } = new ReactiveProperty<double>(0);
        public ReactiveProperty<double> EncoderValue2 { get; set; } = new ReactiveProperty<double>(0);
        public ReactiveProperty<double> EncoderValue3 { get; set; } = new ReactiveProperty<double>(0);
        public ReactiveProperty<double> EncoderValue4 { get; set; } = new ReactiveProperty<double>(0);

        private SerialPortManager m_SerialPortManager;
        private Valve m_Valve;
        private ValveQueue m_ValveQueue;

        // OxyPlotのためのモデルとコントローラー
        public PlotModel Model { get; } = new PlotModel();
        public PlotController Controller { get; } = new PlotController();

        // 軸の設定
        public OxyPlot.Axes.LinearAxis AxisX { get; } = new OxyPlot.Axes.LinearAxis();
        public OxyPlot.Axes.LinearAxis AxisY { get; } = new OxyPlot.Axes.LinearAxis();

        // データを保存するコレクション
        public ObservableCollection<DataPoint> Datas { get; private set; } = new ObservableCollection<DataPoint>();

        // グラフの設定
        public void InitGraph()
        {
            // X軸の設定
            AxisX.Position = OxyPlot.Axes.AxisPosition.Bottom;    // 軸の位置(topにしたら、目盛りが上にくる)
            AxisX.Minimum = 0;
            AxisX.Maximum = 100;

            // Y軸の設定
            AxisY.Position = OxyPlot.Axes.AxisPosition.Left;      // Y軸の位置(Rightにしたら、目盛りが右にくる)
            AxisY.Minimum = sbyte.MinValue;
            AxisY.Maximum = sbyte.MaxValue;

            // 設定した軸をモデルにセット
            Model.Axes.Add(AxisX);
            Model.Axes.Add(AxisY);

            // 線グラフ
            var LineSeries = new OxyPlot.Series.LineSeries();
            LineSeries.Title = "Line";
            LineSeries.Color = OxyColor.FromArgb(0xff, 0xff, 0, 0);     // 上の線の色
            LineSeries.StrokeThickness = 2;                             // 線の太さ
            // データを関連付け
            LineSeries.ItemsSource = Datas;

            // グラフをモデルに追加
            Model.Series.Add(LineSeries);

            // セットした内容を反映させる
            Model.InvalidatePlot(true);
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

            // TODO: 要整理
            // グラフの更新
            Datas.Clear();
            var queue = m_ValveQueue.ReadAll().ToArray();
            var points = new List<DataPoint>();
            for (int i = 0; i < queue.Length; i++)
            {
                points.Add(new DataPoint(i, queue[i].EncoderValue[0]));
            }
            foreach (var point in points)
            {
                Datas.Add(point);
            }
            Model.InvalidatePlot(true);
        }

        public void WriteLine(string value)
        {
            LogData.Value += value + "\r\n";
        }
    }
}
