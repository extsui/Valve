using OxyPlot;
using OxyPlot.Axes;
using OxyPlot.Series;
using Reactive.Bindings;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
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

        public void WriteLine(string value)
        {
            LogData.Value += value + "\r\n";
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

                // 軸の描画
                var axisXMaximum = (double)queue[queue.Length - 1].TimeStamp / 1000;
                var axes = CreateGraphAxis(axisXMaximum);
                foreach (var axis in axes)
                {
                    plotModel.Axes.Add(axis);
                }

                // 線グラフの描画
                var encoderEnabledArray = new bool[] { EncoderEnabled1.Value, EncoderEnabled2.Value, EncoderEnabled3.Value, EncoderEnabled4.Value };
                var lines = CreateGraphLines(queue, encoderEnabledArray);
                foreach (var line in lines)
                {
                    plotModel.Series.Add(line);
                }

                GraphModel = plotModel;
                GraphModel.InvalidatePlot(true);
            }
        }

        // 軸のみの空グラフを初期化時に描画
        private void InitGraph()
        {
            var plotModel = new PlotModel();
            var axes = CreateGraphAxis(5.0);
            foreach (var axis in axes)
            {
                plotModel.Axes.Add(axis);
            }
            GraphModel = plotModel;
        }

        static private List<LinearAxis> CreateGraphAxis(double axisXMaximum)
        {
            var axisX = new LinearAxis();
            var axisY = new LinearAxis();

            // X軸の設定
            axisX.Position = AxisPosition.Bottom;    // 軸の位置(topにしたら、目盛りが上にくる)
            // 50ms 毎にデータが来て 100 個あるので、5秒間のグラフになる
            axisX.Minimum = axisXMaximum - (ValveQueue.CountMax * 0.050);
            axisX.Maximum = axisXMaximum;
            axisX.Title = "時刻 (単位: 秒)";

            // Y軸の設定
            axisY.Position = AxisPosition.Left;      // Y軸の位置(Rightにしたら、目盛りが右にくる)
            axisY.Minimum = sbyte.MinValue;
            axisY.Maximum = sbyte.MaxValue;
            axisY.Title = "エンコーダ値";

            return new List<LinearAxis>() { axisX, axisY };
        }

        static private List<LineSeries> CreateGraphLines(Valve[] queue, bool[] encoderEnabledArray)
        {
            Debug.Assert(encoderEnabledArray.Length == 4);

            // 線グラフの設定
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

            List<(List<DataPoint> points, OxyColor color, bool enabled)> table = new List<(List<DataPoint>, OxyColor, bool)>()
            {
                (points1, OxyColor.FromArgb(0xFF, 246, 173, 60 ), encoderEnabledArray[0]),   // 橙
                (points2, OxyColor.FromArgb(0xFF, 232, 82,  152), encoderEnabledArray[1]),   // 桃
                (points3, OxyColor.FromArgb(0xFF, 0,   169, 95 ), encoderEnabledArray[2]),   // 緑
                (points4, OxyColor.FromArgb(0xFF, 24,  127, 196), encoderEnabledArray[3]),   // 青
            };

            var lines = new List<LineSeries>();
            for (int i = 0; i < table.Count; i++)
            {
                var row = table[i];

                // チェックが付いていないチャンネルは描画しない
                if (!row.enabled)
                {
                    continue;
                }

                var lineSeries = new LineSeries();
                lineSeries.Title = $"#{i + 1}";
                lineSeries.StrokeThickness = 10;
                lineSeries.Color = row.color;
                lineSeries.ItemsSource = row.points;
                lines.Add(lineSeries);
            }

            return lines;
        }
    }
}
