using Reactive.Bindings;
using System;
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
        public ReactiveProperty<string> TxData { get; set; } = new ReactiveProperty<string>();
        public ReactiveProperty<string> RxData { get; set; } = new ReactiveProperty<string>();
        public ReactiveProperty<bool> ButtonConnect_IsEnabled { get; set; } = new ReactiveProperty<bool>(true);
        public ReactiveProperty<bool> ButtonDisconnect_IsEnabled { get; set; } = new ReactiveProperty<bool>(false);

        private SerialPortManager m_SerialPortManager;

        public ViewModel()
        {
            m_SerialPortManager = new SerialPortManager();
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
            RxData.Value += m_SerialPortManager.Read();
        }
    }
}
