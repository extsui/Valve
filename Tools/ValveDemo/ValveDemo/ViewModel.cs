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

        // データを保存するコレクション
        public ObservableCollection<DataPoint> EncoderDatas1 { get; private set; } = new ObservableCollection<DataPoint>();
        public ObservableCollection<DataPoint> EncoderDatas2 { get; private set; } = new ObservableCollection<DataPoint>();
        public ObservableCollection<DataPoint> EncoderDatas3 { get; private set; } = new ObservableCollection<DataPoint>();
        public ObservableCollection<DataPoint> EncoderDatas4 { get; private set; } = new ObservableCollection<DataPoint>();

        // グラフの設定
        public void InitGraph()
        {
            var axisX = new OxyPlot.Axes.LinearAxis();
            var axisY = new OxyPlot.Axes.LinearAxis();

            // X軸の設定
            axisX.Position = OxyPlot.Axes.AxisPosition.Bottom;    // 軸の位置(topにしたら、目盛りが上にくる)
            axisX.Minimum = 0;
            axisX.Maximum = 100;

            // Y軸の設定
            axisY.Position = OxyPlot.Axes.AxisPosition.Left;      // Y軸の位置(Rightにしたら、目盛りが右にくる)
            axisY.Minimum = sbyte.MinValue;
            axisY.Maximum = sbyte.MaxValue;

            // 設定した軸をモデルにセット
            Model.Axes.Add(axisX);
            Model.Axes.Add(axisY);

            // 線グラフ
            var colors = new List<OxyColor>()
            {
                OxyColor.FromArgb(0xFF, 246, 173, 60 ), // 橙
                OxyColor.FromArgb(0xFF, 0,   169, 95 ), // 緑
                OxyColor.FromArgb(0xFF, 232, 82,  152), // 桃
                OxyColor.FromArgb(0xFF, 24,  127, 196), // 青
            };
            var encoderDatas = new List<ObservableCollection<DataPoint>>()
            {
                EncoderDatas1,
                EncoderDatas2,
                EncoderDatas3,
                EncoderDatas4,
            };
            // TODO: Tuple とかでまとめられるハズ
            for (int i = 0; i < 4; i++)
            {
                var lineSeries = new OxyPlot.Series.LineSeries();
                lineSeries.Title = "Line";
                lineSeries.StrokeThickness = 5 *2;
                lineSeries.Color = colors[i];
                lineSeries.ItemsSource = encoderDatas[i];

                // グラフをモデルに追加
                Model.Series.Add(lineSeries);
            }
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

            // グラフの更新
            {
                var queue = m_ValveQueue.ReadAll().ToArray();

                // TODO: 要整理
                EncoderDatas1.Clear();
                EncoderDatas2.Clear();
                EncoderDatas3.Clear();
                EncoderDatas4.Clear();

                var points1 = new List<DataPoint>();
                var points2 = new List<DataPoint>();
                var points3 = new List<DataPoint>();
                var points4 = new List<DataPoint>();
                for (int i = 0; i < queue.Length; i++)
                {
                    points1.Add(new DataPoint(i, queue[i].EncoderValue[0]));
                    points2.Add(new DataPoint(i, queue[i].EncoderValue[1]));
                    points3.Add(new DataPoint(i, queue[i].EncoderValue[2]));
                    points4.Add(new DataPoint(i, queue[i].EncoderValue[3]));
                }
                points1.ForEach(point => EncoderDatas1.Add(point));
                points2.ForEach(point => EncoderDatas2.Add(point));
                points3.ForEach(point => EncoderDatas3.Add(point));
                points4.ForEach(point => EncoderDatas4.Add(point));

                Model.InvalidatePlot(true);
            }
        }

        public void WriteLine(string value)
        {
            LogData.Value += value + "\r\n";
        }
    }
}
