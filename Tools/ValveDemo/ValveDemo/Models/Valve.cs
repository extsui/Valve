using System.Diagnostics;
using System.Linq;
using System.Text.RegularExpressions;
using System.Collections.Generic;

namespace ValveDemo.Models
{
    internal class Valve
    {
        public const int EncoderCount = 4;
        public int TimeStamp;
        public int[] EncoderValue { get; private set; }

        public Valve()
        {
            TimeStamp = 0;
            EncoderValue = new int[EncoderCount];
        }

        public Valve(Valve valve)
        {
            TimeStamp = valve.TimeStamp;
            EncoderValue = (int[])valve.EncoderValue.Clone();
        }

        /// <summary>
        /// Valve からの出力文字列をパースしてエンコーダ値に変換して設定する
        /// </summary>
        public bool Update(string value)
        {
            // 入力例: "32800  -16  -24    8    0"
            var regex = new Regex(" +");
            string[] elements = regex.Replace(value, ",").Split(',');

            var timeStanpString = elements[0];
            int.TryParse(timeStanpString, out TimeStamp);

            var encoderStrings = elements.Skip(1);
            if (encoderStrings.Count() != Valve.EncoderCount)
            {
                return false;
            }

            int[] encoderValue = new int[EncoderCount];
            int count = 0;

            foreach (var str in encoderStrings)
            {
                var result = int.TryParse(str, out encoderValue[count]);
                if (!result)
                {
                    return false;
                }
                count++;
            }

            Debug.Assert(count == EncoderCount);

            for (int i = 0; i < EncoderCount; i++)
            {
                EncoderValue[i] = encoderValue[i];
            }

            return true;
        }
    }

    internal class ValveQueue
    {
        public const int CountMax = 100;

        private Queue<Valve> m_Datas;

        public ValveQueue()
        {
            m_Datas = new Queue<Valve>();
        }

        public void Enqueue(Valve valve)
        {
            m_Datas.Enqueue(valve);
            if (m_Datas.Count > CountMax)
            {
                m_Datas.Dequeue();
            }
        }

        public Queue<Valve> ReadAll()
        {
            return m_Datas;
        }
    }
}
