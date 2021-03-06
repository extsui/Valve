using System;
using System.Collections.Specialized;
using System.Windows;
using System.Windows.Controls;

namespace ValveDemo
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private ViewModel m_ViewModel = new ViewModel();

        public MainWindow()
        {
            InitializeComponent();

            DataContext = m_ViewModel;
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

        private void TextBoxLog_TextChanged(object sender, TextChangedEventArgs e)
        {
            TextBoxLog.ScrollToEnd();
        }

        // リストボックスの自動スクロール処理
        private void ListBoxRxData_TargetUpdated(object sender, System.Windows.Data.DataTransferEventArgs e)
        {
            Action<object, NotifyCollectionChangedEventArgs> collectionChanged = (sender, e) =>
            {
                ListBoxRxData.ScrollIntoView(ListBoxRxData.Items[ListBoxRxData.Items.Count - 1]);
            };

            (ListBoxRxData.ItemsSource as INotifyCollectionChanged).CollectionChanged += new NotifyCollectionChangedEventHandler(collectionChanged);
        }
    }
}
