using System;
using System.Runtime.CompilerServices;

namespace Proton
{
    internal static class InternalCalls
    {
        #region Entity
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static bool Entity_HasComponent(ulong entityID, Type type);
        #endregion
        #region Transform Component
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetPosition(ulong entityID, out Vector3 position);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_SetPosition(ulong entityID, ref Vector3 position);
        #endregion

        #region Input
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsKeyDown(KeyCode keyCode);
        #endregion
    }
}
