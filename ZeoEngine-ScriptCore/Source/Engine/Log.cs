namespace ZeoEngine
{
    public static class Log
    {
        internal enum Level
        {
            Trace,
            Info,
            Warn,
            Error,
            Critical
        }

        public static void Trace(string format, params object[] args)
        {
            InternalCalls.Log_LogMessage(Level.Trace, string.Format(format, args));
        }

        public static void Info(string format, params object[] args)
        {
            InternalCalls.Log_LogMessage(Level.Info, string.Format(format, args));
        }

        public static void Warn(string format, params object[] args)
        {
            InternalCalls.Log_LogMessage(Level.Warn, string.Format(format, args));
        }

        public static void Error(string format, params object[] args)
        {
            InternalCalls.Log_LogMessage(Level.Error, string.Format(format, args));
        }

        public static void Critical(string format, params object[] args)
        {
            InternalCalls.Log_LogMessage(Level.Critical, string.Format(format, args));
        }

    }
}
