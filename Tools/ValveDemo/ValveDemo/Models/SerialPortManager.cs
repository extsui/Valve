using System.IO.Ports;
using System.Text;

namespace ValveDemo.Models
{
    internal class SerialPortManager
    {
        private SerialPort m_SerialPort = new SerialPort();

        public void Open(string port, int baudrate = 115200)
        {
            m_SerialPort.PortName = port;
            m_SerialPort.BaudRate = baudrate;
            m_SerialPort.DataBits = 8;
            m_SerialPort.Parity = Parity.None;
            m_SerialPort.StopBits = StopBits.One;
            m_SerialPort.WriteTimeout = 1000;
            m_SerialPort.ReadTimeout = 1000;
            m_SerialPort.Encoding = Encoding.UTF8;

            m_SerialPort.Open();
        }

        public void Close()
        {
            m_SerialPort.Close();
        }

        public void Write(string data)
        {
            m_SerialPort.Write(data);
        }

        public string Read()
        {
            return m_SerialPort.ReadExisting();
        }

        public void SetReceivedHandler(SerialDataReceivedEventHandler handler)
        {
            m_SerialPort.DataReceived += handler;
        }
    }
}
