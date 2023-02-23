using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Proton;
using static System.Runtime.CompilerServices.RuntimeHelpers;

namespace Sandbox
{
    public class Player : Entity
    {
        TransformComponent m_Transform;
        public float m_Speed = 20.0f;
        public float Time;
        public Entity otherEntity;

        void OnCreate()
        {
            Console.WriteLine($"Player.OnCreate - {ID}");
            m_Transform = GetComponent<TransformComponent>();
        }

        void OnUpdate(float ts)
        {
            //Console.WriteLine($"Player.OnUpdate: {ts}");

            Vector3 velocity = Vector3.Zero;
            Time += ts;

            if (Input.IsKeyDown(KeyCode.A))
                velocity.x -= m_Speed;
            else if (Input.IsKeyDown(KeyCode.D))
                velocity.x += m_Speed;

            if (Input.IsKeyDown(KeyCode.Q))
                velocity.y -= m_Speed;
            else if (Input.IsKeyDown(KeyCode.E))
                velocity.y += m_Speed;

            if (Input.IsKeyDown(KeyCode.W))
                velocity.z += m_Speed;
            else if (Input.IsKeyDown(KeyCode.S))
                velocity.z -= m_Speed;

            Vector3 position = m_Transform.Position;
            position += velocity * ts;
            m_Transform.Position = position;
        }
    }
}
