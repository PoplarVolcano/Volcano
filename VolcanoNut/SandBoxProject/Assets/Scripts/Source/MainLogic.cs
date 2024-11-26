using System;
using Volcano;

namespace Sandbox
{
    public class MainLogic : MonoBehaviour
    {
        public GameObject fireControlObject;
        private void Awake()
        {
            //Application.targetFrameRate = 60;

            InternalCalls.DebugTrace("MainLogic.Awake:" + ID);
        }

        void Start()
        {
            InternalCalls.DebugTrace("MainLogic.Start:" + ID);
        }

        void Update(float ts)
        {
            if(Input.IsMouseButtonClicked(MouseCode.ButtonRight) || Input.IsKeyClicked(KeyCode.F))
            {
                if (fireControlObject != null)
                {
                    FireControl fireControl = fireControlObject.GetComponent<FireControl>();
                    if (fireControl != null)
                    {
                        fireControl.Fire();
                    }
                }
            }

            if (Input.IsKeyClicked(KeyCode.R))
            {
                if (fireControlObject != null)
                {
                    FireControl fireControl = fireControlObject.GetComponent<FireControl>();
                    if(fireControl != null && !fireControl.IsInvoking("Fire"))
                    {
                        fireControl.InvokeRepeating("Fire", 3.0f, 1.0f);
                    }
                }
            }

            if (Input.IsKeyClicked(KeyCode.E))
            {
                if (fireControlObject != null)
                {
                    FireControl fireControl = fireControlObject.GetComponent<FireControl>();
                    if (fireControl != null && fireControl.IsInvoking("Fire"))
                    {
                        fireControl.CancelInvoke("Fire");
                    }
                }
            }

        }

        void OnEnable()
        {
        }

    }
}
