using System;
using System.Collections.Generic;

namespace ZeoEngine
{
    static class TypeUtils
    {
        static bool IsListType(Type type)
        {
            return type.GetGenericTypeDefinition() == typeof(List<>);
        }
    }
}
