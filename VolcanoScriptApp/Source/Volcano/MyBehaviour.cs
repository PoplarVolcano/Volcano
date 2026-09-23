using System;
using System.Runtime.Remoting.Messaging;

namespace Volcano
{
    public class MyBehaviour : Volcano.MonoBehaviour
    {
        public string Message {  get; set; }

        public void Awake()
        {
            Message = "Awake";
            Volcano.InternalCalls.Debug_Trace($"C# says: {Message}");
        }
        public void OnEnable()
        {
            Message = "OnEnable";
            Volcano.InternalCalls.Debug_Trace($"C# says: {Message}");
        }
        public void Start()
        {
            Message = "Start";
            Volcano.InternalCalls.Debug_Trace($"C# says: {Message}");
        }
        public void Update()
        {
            Message = "Update";
            Volcano.InternalCalls.Debug_Trace($"C# says: {Message}");
        }
        public void FixedUpdate()
        {
            Message = "FixedUpdate";
            Volcano.InternalCalls.Debug_Trace($"C# says: {Message}");
        }
        public void LateUpdate()
        {
            Message = "LateUpdate";
            Volcano.InternalCalls.Debug_Trace($"C# says: {Message}");
        }
        public void OnDisable()
        {
            Message = "OnDisable";
            Volcano.InternalCalls.Debug_Trace($"C# says: {Message}");
        }
        public void OnDistroy()
        {
            Message = "OnDistroy";
            Volcano.InternalCalls.Debug_Trace($"C# says: {Message}");
        }
    }
}
