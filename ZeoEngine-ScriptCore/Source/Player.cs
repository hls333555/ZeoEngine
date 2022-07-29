using System;
using ZeoEngine;

namespace Sandbox
{
    public class Player : Entity
    {
        void OnCreate()
        {
            Console.WriteLine($"Player.OnCreate - {ID}");
        }

        void OnUpdate(float dt)
        {
            Console.WriteLine($"Player.OnUpdate: {dt}");

            Vector3 translation = Translation;
            if (Input.IsKeyPressed(KeyCode.W))
            {
                translation.Y += dt;
            }
            if (Input.IsKeyPressed(KeyCode.S))
            {
                translation.Y -= dt;
            }
            if (Input.IsKeyPressed(KeyCode.A))
            {
                translation.X -= dt;
            }
            if (Input.IsKeyPressed(KeyCode.D))
            {
                translation.X += dt;
            }
            Translation = translation;
        }
    }
}
