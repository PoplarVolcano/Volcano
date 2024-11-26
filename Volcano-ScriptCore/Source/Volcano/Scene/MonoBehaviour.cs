using System;

namespace Volcano
{
    // MonoBehaviour is a base class that many Volcano scripts derive from.
    public class MonoBehaviour : Behaviour
    {

        // Is any invoke pending on this Entity?
        public bool IsInvoking()
        {
            return InternalCalls.MonoBehaviour_IsInvokingAll(ID);
        }

        public bool IsInvoking(string methodName)
        {
            return InternalCalls.MonoBehaviour_IsInvoking(ID, methodName);
        }

        public void CancelInvoke()
        {
            InternalCalls.MonoBehaviour_CancelInvokeAll(ID);
        }

        // Cancels all Invoke calls with name /methodName/ on this behaviour.
        public void CancelInvoke(string methodName)
        {
            InternalCalls.MonoBehaviour_CancelInvoke(ID, methodName);
        }

        public void Invoke(string methodName, float time)
        {
            InternalCalls.MonoBehaviour_InvokeDelayed(ID, methodName, time, 0.0f);
        }

        public void InvokeRepeating(string methodName, float time, float repeatRate)
        {
            if (repeatRate <= 0.00001f && repeatRate != 0.0f)
                return;
            //throw new UnityException("Invoke repeat rate has to be larger than 0.00001F");

            InternalCalls.MonoBehaviour_InvokeDelayed(ID, methodName, time, repeatRate);
        }

    }
}
