using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

namespace Volcano
{
    public class Entity
    {
        public readonly ulong ID;
        public Entity() { ID = 0; }
        internal Entity(ulong id) { ID = id; }


        const string entityIsNullMessage = "The Entity you want to instantiate is null.";
        const string cloneDestroyedMessage = "Instantiate failed because the clone was destroyed during creation. This can happen if DestroyImmediate is called in Awake().";

        public static bool IsEntityExist(Entity entity)
        {
            return InternalCalls.Entity_IsEntityExist(entity.ID);
        }

        // Clones the Entity /original/ and returns the clone.
        public static Entity Instantiate(Entity original, Vector3 position, Quaternion rotation)
        {
            if (original == null)
            {
                InternalCalls.DebugError(entityIsNullMessage);
                return null;
            }

            var entity = InternalCalls.Entity_InstantiateSingle(original, position, rotation);

            if (entity == null)
                InternalCalls.DebugError(cloneDestroyedMessage);

            return entity;
        }
        // Clones the Entity /original/ and returns the clone.
        public static Entity Instantiate(Entity original, Vector3 position, Quaternion rotation, Transform parent)
        {
            if (parent == null)
                return Instantiate(original, position, rotation);

            if (original == null)
            {
                InternalCalls.DebugError(entityIsNullMessage);
                return null;
            }

            var entity = InternalCalls.Entity_InstantiateSingleWithParent(original, parent, position, rotation);

            if (entity == null)
                InternalCalls.DebugError(cloneDestroyedMessage);

            return entity;
        }

        // Clones the Entity /original/ and returns the clone.
        public static Entity Instantiate(Entity original)
        {
            if (original == null)
            {
                InternalCalls.DebugError(entityIsNullMessage);
                return null;
            }

            var entity = InternalCalls.Entity_CloneSingle(original);

            if (entity == null)
                InternalCalls.DebugError(cloneDestroyedMessage);

            return entity;
        }

        // Clones the Entity /original/ and returns the clone.
        public static Entity Instantiate(Entity original, Transform parent)
        {
            return Instantiate(original, parent, false);
        }

        public static Entity Instantiate(Entity original, Transform parent, bool instantiateInWorldSpace)
        {
            if (parent == null)
                return Instantiate(original);

            if (original == null)
            {
                InternalCalls.DebugError(entityIsNullMessage);
                return null;
            }

            var entity = InternalCalls.Entity_CloneSingleWithParent(original, parent, instantiateInWorldSpace);

            if (entity == null)
                InternalCalls.DebugError(cloneDestroyedMessage);

            return entity;
        }

        
        public static T Instantiate<T>(T original) where T : Entity
        {
            if (original == null)
            {
                InternalCalls.DebugError(entityIsNullMessage);
                return null;
            }

            var entity = (T)InternalCalls.Entity_CloneSingle(original);

            if (entity == null)
                InternalCalls.DebugError(cloneDestroyedMessage);

            return entity;
        }

        public static T Instantiate<T>(T original, Vector3 position, Quaternion rotation) where T : Entity
        {
            return (T)Instantiate((Entity)original, position, rotation);
        }

        public static T Instantiate<T>(T original, Vector3 position, Quaternion rotation, Transform parent) where T : Entity
        {
            return (T)Instantiate((Entity)original, position, rotation, parent);
        }

        public static T Instantiate<T>(T original, Transform parent) where T : Entity
        {
            return Instantiate<T>(original, parent, false);
        }

        public static T Instantiate<T>(T original, Transform parent, bool worldPositionStays) where T : Entity
        {
            return (T)Instantiate((Entity)original, parent, worldPositionStays);
        }

        // Removes a gameobject, component or asset.
        public static void Destroy(Entity entity)
        {
            float time = 0.0F;
            InternalCalls.Entity_Destroy(entity, time);
        }


    }

}