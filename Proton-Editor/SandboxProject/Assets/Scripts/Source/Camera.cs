using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Proton;
using static System.Runtime.CompilerServices.RuntimeHelpers;

namespace Sandbox
{
    public class Camera : Entity
    {
        public float maxDist = 5.0f;

        Entity player;
        void OnCreate()
        {
            Console.WriteLine("Created camera entity");
            player = FindEntityByName("Player");
        }
        void OnUpdate(float ts)
        {
            //Console.WriteLine($"Player.OnUpdate: {ts}");
            if (player != null)
            {
                Player playerClass = player.As<Player>();
                if((player.Position - Position).SqrMagnitude() <= maxDist * maxDist)
                {
                    Position = Vector3.Normalize(Position - player.Position) * maxDist + player.Position;
                }

                playerClass.m_Speed += ts;
            }
            else
            {
                Console.WriteLine("Player is null!");
            }

            float speed = 100.0f;
            Vector3 velocity = Vector3.Zero;

            if (Input.IsKeyDown(KeyCode.A))
                velocity.x -= speed;
            else if (Input.IsKeyDown(KeyCode.D))
                velocity.x += speed;

            if (Input.IsKeyDown(KeyCode.Q))
                velocity.y -= speed;
            else if (Input.IsKeyDown(KeyCode.E))
                velocity.y += speed;

            if (Input.IsKeyDown(KeyCode.W))
                velocity.z += speed;
            else if (Input.IsKeyDown(KeyCode.S))
                velocity.z -= speed;

            Vector3 position = Position;
            position += velocity * ts;
            //Position = position;
        }
    }
}
