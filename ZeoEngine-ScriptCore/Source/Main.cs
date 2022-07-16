using System;

namespace ZeoEngine
{
    public class Main
    {
        public float FloatVar { get; set; }

        public Main()
        {
            Console.WriteLine("Main ctor!");
        }

        public void PrintMsg()
        {
            Console.WriteLine("Hellow world from C#!");
        }

        public void PrintInt(int value)
        {
            Console.WriteLine($"PrintInt: {value}");
        }

        public void PrintInts(int value1, int value2)
        {
            Console.WriteLine($"PrintInts: {value1}, {value2}");
        }

        public void PrintString(string value)
        {
            Console.WriteLine($"PrintString: {value}");
        }
    }
}
