using System.Diagnostics;
using System.Linq;
using System.Text.RegularExpressions;

namespace ValveDemo.Models
{
    internal class Valve
    {
        public const int EncoderCount = 4;
        public int[] EncoderValue { get; private set; }

        public Valve()
        {
            EncoderValue = new int[EncoderCount];
        }

        /// <summary>
        /// Valve からの出力文字列をパースしてエンコーダ値に変換して設定する
        /// </summary>
        public bool Update(string value)
        {
            // 入力例: "32800  -16  -24    8    0"
            var regex = new Regex(" +");
            string[] elements = regex.Replace(value, ",").Split(',');

            var encoderStrings = elements.Skip(1);

            if (encoderStrings.Count() != Valve.EncoderCount)
            {
                return false;
            }

            int[] encoderValue = new int[EncoderCount];
            int count = 0;

            foreach (var str in encoderStrings)
            {
                int encoder = 0;
                var result = int.TryParse(str, out encoder);
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
}
