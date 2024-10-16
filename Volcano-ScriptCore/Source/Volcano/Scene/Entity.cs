﻿using System;
using System.Runtime.CompilerServices;

namespace Volcano
{
    public class Entity
    {
        public readonly ulong ID;
        protected Entity() { ID = 0; }
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

    }

}