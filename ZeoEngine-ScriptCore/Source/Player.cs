using System;
using ZeoEngine;

namespace Sandbox
{
    public class Player : Entity
    {
        void OnCreate()
        {
            Console.WriteLine($"Entity: {Name}");
        }

        void OnUpdate(float dt)
        {
            Vector3 translation = Translation;
            if (Input.IsKeyPressed(KeyCode.W))
            {
                translation += GetForwardVector() * dt;
            }
            if (Input.IsKeyPressed(KeyCode.S))
            {
                translation -= GetForwardVector() * dt;
            }
            if (Input.IsKeyPressed(KeyCode.A))
            {
                translation -= GetRightVector() * dt;
            }
            if (Input.IsKeyPressed(KeyCode.D))
            {
                translation += GetRightVector() * dt;
            }
            Translation = translation;
        }
    }
}
