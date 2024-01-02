// script to communicate with dVRK using UDP


using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.IO;



public class UDPComm : MonoBehaviour
{

    // for udp socket connection
    public static byte[] data;
    public static Socket socket;
    public static EndPoint remote;
    public static byte[] send_msg;
    public static string pose_message;
    public static string jaw_message;
    public static string dVRK_msg;
    public static bool jaw_match;
    public static int read_msg_count = 0;

    public static float jaw_angle;    // read in jaw angle from dVRK
    public static Quaternion EE_quat;
    public static Vector3 EE_pos;  // EE pos from dVRK
    bool ReaddVRKmsg = false;
    int readcounter = 0;
    float timer;
    int filename = 0;
    float delta_timer_holo;
    float timer_holo;

    public GameObject hololens;
    public TextMesh read;   // text for recording

    bool pause = false;
    string file_name_dVRK;
    string file_name_holo;

    public GameObject straight_wire;
    public GameObject s_wire;
    public GameObject angled_wire;

    // Start is called before the first frame update
    void Start()
    {
        // udp using socket
        data = new byte[1024];
        IPEndPoint ip = new IPEndPoint(IPAddress.Any, 48051);
        socket = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);
        socket.Bind(ip);
        IPEndPoint sender = new IPEndPoint(IPAddress.Any, 0);
        remote = (EndPoint)(sender);
        //Debug.Log("start again");
    }

    // Update is called once per frame
    void Update()
    {
        if (Main.MTM == true)
        {
            Debug.Log("Main.MTM");
            if (straight_wire.activeSelf)
            {
                 file_name_dVRK = filename + "_MTM_Omni_straight";
                file_name_holo = filename + "_MTM_Omni_straight";
            }

            if(s_wire.activeSelf)
            {
                file_name_dVRK = filename + "_MTM_Omni_curved";
                file_name_holo = filename + "_MTM_Omni_curved";
            }

            if (angled_wire.activeSelf)
            {
                file_name_dVRK = filename + "_MTM_Omni_angled";
                file_name_holo = filename + "_MTM_Omni_angled";
            }

        }
        else
                {
            if (straight_wire.activeSelf)
            {
                file_name_dVRK = filename + "_Hand_straight";
                file_name_holo = filename + "_Hand_straight";
            }

            else if (s_wire.activeSelf)
            {
                file_name_dVRK = filename + "_Hand_curved";
                file_name_holo = filename + "_Hand_curved";
            }

            else if (angled_wire.activeSelf)
            {
                file_name_dVRK = filename + "_Hand_angled";
                file_name_holo = filename + "_Hand_angled";
            }
            else
            {

                file_name_dVRK = filename + "_Hand_drawn";
                file_name_holo = filename + "_Hand_drawn";

            }




        }

        string incoming_pose = Path.Combine(Application.persistentDataPath, "dVRKpose_" + file_name_dVRK + ".txt");
        //string incoming_jaw = Path.Combine(Application.persistentDataPath, "dVRKjaw_" + filename + ".txt");
        string hololens_transform_pinch = Path.Combine(Application.persistentDataPath, "hololens_" + file_name_holo + ".txt");

        read_msg_count = 0;

        while (read_msg_count < 2)
        {
            // read in message from dVRK
            data = new byte[1024];
            socket.ReceiveFrom(data, ref remote);
            dVRK_msg = Encoding.UTF8.GetString(data);
            // if jaw message
            if (parser.StringMatch(dVRK_msg, "\"jaw/setpoint_js\":"))
            {
                // extract jaw angle
                jaw_angle = parser.GetJawAngle(dVRK_msg);

                // reocord data
                //if (ReaddVRKmsg)
                //{
                //    using (StreamWriter writer = new StreamWriter(incoming_jaw, true))
                //    {

                //        timer += Time.deltaTime;
                //        string jaw = "\n " + timer + " " + jaw_angle.ToString("R");
                //        writer.WriteLine(jaw);

                //    }
                //}
            }

            // if pose message
            else if (parser.StringMatch(dVRK_msg, "\"setpoint_cp\":"))
            {
                // extract rot and pos
                EE_quat = QuaternionFromMatrix(parser.GetMatrix4X4(dVRK_msg));
                Matrix4x4 temp = parser.GetMatrix4X4(dVRK_msg);
                //Debug.Log("dVRK rot: " + temp.rotation);
                EE_pos = parser.GetPos(dVRK_msg);

                // record data
                if (ReaddVRKmsg)
                {
                    Debug.Log(pause);
                    if (!pause)
                    {
                        read.text = "Recording";
                        using (StreamWriter writer = new StreamWriter(incoming_pose, true))
                        {
                            timer += Time.deltaTime;
                            string pose = "\n " + timer + " " + EE_pos[0].ToString("R") + " " + EE_pos[1].ToString("R") + " " + EE_pos[2].ToString("R") + " " + EE_quat[0].ToString("R") + " " + EE_quat[1].ToString("R") + " " + EE_quat[2].ToString("R") + " " + EE_quat[3].ToString("R");
                            writer.WriteLine(pose);

                        }

                        using (StreamWriter writer = new StreamWriter(hololens_transform_pinch, true))
                        {
                            delta_timer_holo += Time.deltaTime;
                            timer_holo = Time.unscaledDeltaTime;
                            string holo_transform = "\n" + timer_holo + " " + timer + " " + hololens.transform.position[0].ToString("R") + " " + hololens.transform.position[1].ToString("R") + " " + hololens.transform.position[2].ToString("R") + " " + hololens.transform.rotation[0].ToString("R") + " " + hololens.transform.rotation[1].ToString("R") + " " + hololens.transform.rotation[2].ToString("R") + " " + hololens.transform.rotation[3].ToString("R");
                            writer.WriteLine(holo_transform);
                        }
                    }
                    else
                    {
                        Debug.Log("paused");
                        read.text = "Recording paused";
                    }
                }
                else
                {
                    read.text = "Recording complete";
                }
            }

            // if neither, probably restarted
            else
            {
                HandTrack.new_EE_pos = Vector3.zero;
            }

            read_msg_count += 1;
        }
    }


    // get quaternion from homogeneous matrix
    public static Quaternion QuaternionFromMatrix(Matrix4x4 m)
    {
        return Quaternion.LookRotation(m.GetColumn(2), m.GetColumn(1));
    }

    public void ReaddVRK()
    {
        readcounter++;
        if (readcounter > 1)
        {
            ReaddVRKmsg = false;
            Debug.Log("stop Reading dVRK");
            readcounter = 0;

            //read.text = "Recording complete";

            read.GetComponent<Renderer>().enabled = true;

        }
        else
        {

            ReaddVRKmsg = true;
            Debug.Log("Reading dVRK");
            readcounter++;
            filename++;
            //read.text = "Recording";

            read.GetComponent<Renderer>().enabled = true;
        }
    }

    // sends pose and jaw messages to dVRK over UDP connection
    public void UDPsend(string pose_message, string jaw_message)
    {
        // send json strings to dVRK //
        send_msg = Encoding.UTF8.GetBytes(pose_message);
        socket.SendTo(send_msg, remote);
        send_msg = Encoding.UTF8.GetBytes(jaw_message);
        socket.SendTo(send_msg, remote);
    }

    public void PauseRecord()
    {
        pause = !pause;
    }

}
