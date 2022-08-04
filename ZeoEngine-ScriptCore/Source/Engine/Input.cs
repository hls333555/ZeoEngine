namespace ZeoEngine
{
    public class Input
    {
        public static bool IsKeyPressed(KeyCode keycode)
        {
            return InternalCalls.Input_IsKeyPressed(keycode);
        }

        public static bool IsKeyReleased(KeyCode keycode)
        {
            return InternalCalls.Input_IsKeyReleased(keycode);
        }

        public static bool IsKeyPressed(MouseCode mousecode)
        {
            return InternalCalls.Input_IsMouseButtonPressed(mousecode);
        }

        public static bool IsKeyReleased(MouseCode mousecode)
        {
            return InternalCalls.Input_IsMouseButtonReleased(mousecode);
        }
    }
}
