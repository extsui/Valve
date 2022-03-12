using OxyPlot;
using OxyPlot.Series;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Windows;

namespace ValveDemo
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private ViewModel m_ViewModel = new ViewModel();

        // OxyPlotのためのモデルとコントローラー
        public PlotModel Model { get; } = new PlotModel();
        public PlotController Controller { get; } = new PlotController();

        // 軸の設定
        public OxyPlot.Axes.LinearAxis AxisX { get; } = new OxyPlot.Axes.LinearAxis();
        public OxyPlot.Axes.LinearAxis AxisY { get; } = new OxyPlot.Axes.LinearAxis();

        // データを保存するコレクション
        public ObservableCollection<DataPoint> Datas { get; private set; } = new ObservableCollection<DataPoint>();

        public MainWindow()
        {
            InitializeComponent();
            InitGraph();

            DataContext = m_ViewModel;
        }

        // グラフの設定
        public void InitGraph()
        {
            // X軸の設定
            AxisX.Position = OxyPlot.Axes.AxisPosition.Bottom;    // 軸の位置(topにしたら、目盛りが上にくる)
            AxisX.Minimum = 0;
            AxisX.Maximum = 100;
            
            // Y軸の設定
            AxisY.Position = OxyPlot.Axes.AxisPosition.Left;      // Y軸の位置(Rightにしたら、目盛りが右にくる)
            AxisY.Minimum = -1;
            AxisY.Maximum = 1;

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
            // 点を追加
            var rand = new Random();
            List<DataPoint> points = new List<DataPoint>();
            // 適当なサンプルデータを設定
            for (int i = 0; i < 100; i++)
            {
                points.Add(new DataPoint(i, (rand.NextDouble() * 2 - 1) * 0.1));
            }
            foreach (var point in points)
            {
                Datas.Add(point);
            }

            // グラフをモデルに追加
            Model.Series.Add(LineSeries);

            // セットした内容を反映させる
            Model.InvalidatePlot(true);
        }

        private void ComboBoxSerialPort_DropDownOpened(object sender, EventArgs e)
        {
            m_ViewModel.GetComPorts();
        }

        private void ButtonConnect_Click(object sender, RoutedEventArgs e)
        {
            m_ViewModel.SerialOpen();
        }

        private void ButtonDisconnect_Click(object sender, RoutedEventArgs e)
        {
            m_ViewModel.SerialClose();
        }
    }
}
