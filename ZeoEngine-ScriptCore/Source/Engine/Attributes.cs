using System;

namespace ZeoEngine.Attributes
{
    [System.AttributeUsage(System.AttributeTargets.Field)]
    public class Tooltip : Attribute
    {
        private string m_Value;

        public Tooltip(string tooltip)
        {
            m_Value = tooltip;
        }
    }

    [System.AttributeUsage(System.AttributeTargets.Field)]
    public class Category : Attribute
    {
        private string m_Value;

        public Category(string category)
        {
            m_Value = category;
        }
    }

    [System.AttributeUsage(System.AttributeTargets.Field)]
    public class HiddenInEditor : Attribute
    {
    }

    //[System.AttributeUsage(System.AttributeTargets.Field)]
    //public class HideCondition : Attribute
    //{
    //    private string m_Value;

    //    public HideCondition(string func)
    //    {
    //        m_Value = func;
    //    }
    //}

    [System.AttributeUsage(System.AttributeTargets.Field)]
    public class Transient : Attribute
    {
    }

    [System.AttributeUsage(System.AttributeTargets.Field)]
    public class DragSensitivity : Attribute
    {
        private float m_Value;

        public DragSensitivity(float speed)
        {
            m_Value = speed;
        }
    }

    [System.AttributeUsage(System.AttributeTargets.Field)]
    public class ClampMin : Attribute
    {
        private float m_Value;

        public ClampMin(float min)
        {
            m_Value = min;
        }
    }

    [System.AttributeUsage(System.AttributeTargets.Field)]
    public class ClampMax : Attribute
    {
        private float m_Value;

        public ClampMax(float max)
        {
            m_Value = max;
        }
    }

    [System.AttributeUsage(System.AttributeTargets.Field)]
    public class ClampOnlyDuringDragging : Attribute
    {
    }

    [System.AttributeUsage(System.AttributeTargets.Field)]
    public class AssetType : Attribute
    {
        private Type m_Value;

        public AssetType(Type type)
        {
            m_Value = type;
        }
    }

}
