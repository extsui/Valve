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
        public ReactiveProperty<string> RxData { get; set; } = new ReactiveProperty<string>();
        public ReactiveProperty<string> LogData { get; set; } = new ReactiveProperty<string>();
        public ReactiveProperty<bool> ButtonConnect_IsEnabled { get; set; } = new ReactiveProperty<bool>(true);
        public ReactiveProperty<bool> ButtonDisconnect_IsEnabled { get; set; } = new ReactiveProperty<bool>(false);

        public ReactiveProperty<double> EncoderValue1 { get; set; } = new ReactiveProperty<double>(0);
        public ReactiveProperty<double> EncoderValue2 { get; set; } = new ReactiveProperty<double>(0);
        public ReactiveProperty<double> EncoderValue3 { get; set; } = new ReactiveProperty<double>(0);
        public ReactiveProperty<double> EncoderValue4 { get; set; } = new ReactiveProperty<double>(0);

        private SerialPortManager m_SerialPortManager;
        private Valve m_Valve;

        public ViewModel()
        {
            m_SerialPortManager = new SerialPortManager();
            m_Valve = new Valve();

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
            RxData.Value += value + "\r\n";

            var result = m_Valve.Update(value);
            if (!result)
            {
                WriteLine($"[warn] valve string error! (\"{value}\")");
            }

            EncoderValue1.Value = (double)m_Valve.EncoderValue[0];
            EncoderValue2.Value = (double)m_Valve.EncoderValue[1];
            EncoderValue3.Value = (double)m_Valve.EncoderValue[2];
            EncoderValue4.Value = (double)m_Valve.EncoderValue[3];
        }

        public void WriteLine(string value)
        {
            LogData.Value += value + "\r\n";
        }
    }
}
