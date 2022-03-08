﻿using OxyPlot;
using OxyPlot.Series;
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
            List<DataPoint> points = new List<DataPoint>()
            {
                // TORIAEZU: サンプルデータ。要削除
                // (x, y)
                new DataPoint(0, 0),
                new DataPoint(1, 0.5),
                new DataPoint(2, 0),
                new DataPoint(3, -0.5),
                new DataPoint(4, -1),
            };
            foreach (var point in points)
            {
                Datas.Add(point);
            }

            // グラフをモデルに追加
            Model.Series.Add(LineSeries);

            // セットした内容を反映させる
            Model.InvalidatePlot(true);
        }
    }
}