using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

namespace Volcano
{
    public class Entity
    {
        public readonly ulong ID;
        public Entity() { ID = 0; }
        internal Entity(ulong id) 
        { 
            ID = id;
            transform = GetComponent<TransformComponent>();
        }

        public TransformComponent transform;

        public bool HasComponent<T>() where T : Component, new()
        {
            Type componentType = typeof(T);
            return InternalCalls.Entity_HasComponent(ID, componentType);
        }

        public T GetComponent<T>() where T : Component, new()
        {
            if (!HasComponent<T>())
                return null;

            T component = new T() { Entity = this };
            return component;
        }

        public Entity FindEntityByName(string name)
        {
            // 通过实体名获得实体ID，注：未调用脚本的实体无法在脚本中获得
            ulong entityID = InternalCalls.Entity_FindEntityByName(name);
            if (entityID == 0)
                return null;

            return new Entity(entityID);
        }

        public T As<T>() where T : Entity, new()
        {
            object instance = InternalCalls.GetScriptInstance(ID);
            return instance as T;
        }


        // The parent of the Entity.
        public Entity parent
        {
            get { return new Entity(InternalCalls.Entity_GetParent(ID)); }
        }

        public ulong childCount
        {
            get => InternalCalls.Entity_GetChildrenCount(ID);
        }

        public Entity[] children
        {
            get {
                InternalCalls.Entity_GetChildren(ID, out ulong[] result);
                List<ulong> ids = new List<ulong>();
                foreach (ulong id in result)
                {
                    if (InternalCalls.Entity_IsEntityExist(id))
                        ids.Add(id);
                }
                Entity[] children = new Entity[ids.Count];
                if (result != null)
                    for (int i = 0; i < ids.Count; i++)
                    {
                            children[i] = new Entity(ids[i]);
                    }
                return children;
            }
        }

        public Entity GetChild(ulong index)
        {
            ulong childId = InternalCalls.Entity_GetChild(ID, index);
            if(childId == 0)
                return null;
            else
                return new Entity(childId);
        }

        // Is any invoke pending on this Entity?
        public bool IsInvoking()
        {
            return InternalCalls.IsInvokingAll(ID);
        }

        public bool IsInvoking(string methodName)
        {
            return InternalCalls.IsInvoking(ID, methodName);
        }

        public void CancelInvoke()
        {
            InternalCalls.CancelInvokeAll(ID);
        }

        // Cancels all Invoke calls with name /methodName/ on this behaviour.
        public void CancelInvoke(string methodName)
        {
            InternalCalls.CancelInvoke(ID, methodName);
        }

        public void Invoke(string methodName, float time)
        {
            InternalCalls.InvokeDelayed(ID, methodName, time, 0.0f);
        }

        public void InvokeRepeating(string methodName, float time, float repeatRate)
        {
            if (repeatRate <= 0.00001f && repeatRate != 0.0f)
                return;
                //throw new UnityException("Invoke repeat rate has to be larger than 0.00001F");

            InternalCalls.InvokeDelayed(ID, methodName, time, repeatRate);
        }

        const string entityIsNullMessage = "The Entity you want to instantiate is null.";
        const string cloneDestroyedMessage = "Instantiate failed because the clone was destroyed during creation. This can happen if DestroyImmediate is called in Awake().";

        // Clones the Entity /original/ and returns the clone.
        public static Entity Instantiate(Entity original, Vector3 position, Quaternion rotation)
        {
            if (original == null)
            {
                InternalCalls.DebugError(entityIsNullMessage);
                return null;
            }

            var entity = InternalCalls.InstantiateSingle(original, position, rotation);

            if (entity == null)
                InternalCalls.DebugError(cloneDestroyedMessage);

            return entity;
        }
        // Clones the Entity /original/ and returns the clone.
        public static Entity Instantiate(Entity original, Vector3 position, Quaternion rotation, TransformComponent parent)
        {
            if (parent == null)
                return Instantiate(original, position, rotation);

            if (original == null)
            {
                InternalCalls.DebugError(entityIsNullMessage);
                return null;
            }

            var entity = InternalCalls.InstantiateSingleWithParent(original, parent, position, rotation);

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

            var entity = InternalCalls.CloneSingle(original);

            if (entity == null)
                InternalCalls.DebugError(cloneDestroyedMessage);

            return entity;
        }

        // Clones the Entity /original/ and returns the clone.
        public static Entity Instantiate(Entity original, TransformComponent parent)
        {
            return Instantiate(original, parent, false);
        }

        public static Entity Instantiate(Entity original, TransformComponent parent, bool instantiateInWorldSpace)
        {
            if (parent == null)
                return Instantiate(original);

            if (original == null)
            {
                InternalCalls.DebugError(entityIsNullMessage);
                return null;
            }

            var entity = InternalCalls.CloneSingleWithParent(original, parent, instantiateInWorldSpace);

            if (entity == null)
                InternalCalls.DebugError(cloneDestroyedMessage);

            return entity;
        }

        /*
        public static T Instantiate<T>(T original) where T : Entity
        {
            if (original == null)
            {
                InternalCalls.DebugError(entityIsNullMessage);
                return null;
            }

            var entity = (T)InternalCalls.CloneSingle(original);

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
        */

    }

}