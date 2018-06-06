#include "server.h"
#include <iostream>
#include <string>
#include <string.h>

using namespace paxos;
using namespace std;

int parse_ipport(const char * pcStr, paxos::NodeInfo & oNodeInfo) {
    char sIP[32] = {0};
    int iPort = -1;

    int count = sscanf(pcStr, "%[^':']:%d", sIP, &iPort);
    if (count != 2) {
        return -1;
    }
    oNodeInfo.SetIpPort(sIP, iPort);

    return 0;
}

int parse_ipport_list(const char * pcStr, NodeInfoList & vec_node_info_list) {
    string sTmpStr;
    int iStrLen = strlen(pcStr);

    for (int i = 0; i < iStrLen; i++) {
        if (pcStr[i] == ',' || i == iStrLen - 1) {
            if (i == iStrLen - 1 && pcStr[i] != ',') {
                sTmpStr += pcStr[i];
            }

            paxos::NodeInfo oNodeInfo;
            int ret = parse_ipport(sTmpStr.c_str(), oNodeInfo);
            if (ret != 0) {
                return ret;
            }

            vec_node_info_list.push_back(oNodeInfo);

            sTmpStr = "";
        }
        else {
            sTmpStr += pcStr[i];
        }
    }

    return 0;
}

int main(int argc, char ** argv) {
    char* ip_local = "127.0.0.1:11111";
    char* ip_list = "127.0.0.1:11111,127.0.0.1:11112,127.0.0.1:11113";

    if (argc < 3) {
        cout << "wtf, insufficient args" << endl;
        //return -1;
    }

    argv[1] = ip_local;
    argv[2] = ip_list;

    paxos::NodeInfo node;
    if (parse_ipport(argv[1], node) != 0) {
        cout << "wtf, parsing fails" << endl;
        return -1;
    }

    NodeInfoList vec_node_info_list;
    if (parse_ipport_list(argv[2], vec_node_info_list) != 0) {
        cout << "wtf, parsing fails again" << endl;
        return -1;
    }

    PhxEchoServer server(node, vec_node_info_list);
    int ret = server.Run();
    if (ret != 0) {
        return -1;
    }

    printf("echo server start, ip %s port %d\n", node.GetIp().c_str(), node.GetPort());

    string echo_send_val;
    while (true) {
        printf("\nplease input: <echo req value>\n");
        getline(cin, echo_send_val);
        string sEchoRespValue;
        ret = server.Echo(echo_send_val, sEchoRespValue);
        if (ret != 0) {
            printf("Echo fail, ret %d\n", ret);
        }
        else {
            printf("echo resp value %s\n", sEchoRespValue.c_str());
        }
    }
}
