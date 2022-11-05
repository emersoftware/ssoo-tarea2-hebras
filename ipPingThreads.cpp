#include <thread>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstring>
using namespace std;
//Emerson Benjamín Salazar Rubilar

//Create a class to hold the data for each ip
class ipData{
public:
    string ip;
    int packetsTransmitted;
    int packetsReceived;
    int packetsLost;
    string status;

    //Default constructor
    ipData(){
        ip = "";
        packetsTransmitted = 0;
        packetsReceived = 0;
        packetsLost = 0;
        status = "Down";
    }

    //Setter all data
    void setData(string ip, int packetsTransmitted, int packetsReceived, int packetsLost, string status){
        this->ip = ip;
        this->packetsTransmitted = packetsTransmitted;
        this->packetsReceived = packetsReceived;
        this->packetsLost = packetsLost;
        this->status = status;
    }

};

string pingCommand(string ip, string packets){
  string command = "ping -c " + packets + " " + ip;
  string result = "";

  //Ping the ip and get the result in result variable
  FILE *in;
  char buff[512];
  if(!(in = popen(command.c_str(), "r"))){
    return "";
  }
  while(fgets(buff, sizeof(buff), in)!=NULL){
    result += buff;
  }
  pclose(in);
  return result;
}

void pingIp(string ip, string packets, ipData *ipData){
  string result = pingCommand(ip, packets);
  if(result==""){
    cout << "Error: No se pudo ejecutar el comando ping." << ip << endl;
    return;
  }
  //Get data from the result
  int packetsTransmitted = 0;
  int packetsReceived = 0;
  int packetsLost = 0;
  string status = "Down";
  string line;
  istringstream iss(result);
  while(getline(iss, line)){
    if(line.find("packets transmitted") != string::npos){
      packetsTransmitted = stoi(line.substr(0, line.find(" ")));
    }
    if(line.find("received") != string::npos){
      packetsReceived = stoi(line.substr(line.find("transmitted, ")+13, line.find(" received")-line.find("transmitted, ")+13));
    }
    if(line.find("packet loss") != string::npos){
      //Get the number of packets lost considering the number of packets transmitted and packetsLostPorcentage variable which is the porcentage of packets lost.
      packetsLost = stoi(line.substr(line.find("received, ")+10, line.find(" packet loss")-line.find("received, ")+10))/100*packetsTransmitted;
    }
  }
  if(packetsReceived == packetsTransmitted){
    status = "Up";
  }

  //Set the data in the ipData object
  ipData->setData(ip, packetsTransmitted, packetsReceived, packetsLost, status);
  
  //cout << "IP: " << ip << "Packets: Sent = " << packetsTransmitted << ", Received = " << packetsReceived << ", Lost = " << packetsLost << " , Status = " << status << endl;


}


int main(int argc, char *argv[]) {
  //Check if the number of arguments is correct
  if(argv[1] == NULL){
    cout << "Error: No se ingreso el archivo de ips." << endl;
    cout << "Uso: ./pingIpThreads archivo.txt numerodepaquetes"<< endl;
    return 0;
  }

  if (argv[2] == NULL){
    cout << "Error: No se ingreso el numero de paquetes." << endl;
    cout << "Uso: ./pingIpThreads archivo.txt numerodepaquetes"<< endl;
    return 0;
  }

  if (argc > 3){
    cout << "Error: Demasiados argumentos." << endl;
    cout << "Uso: ./pingIpThreads archivo.txt numerodepaquetes"<< endl;
    return 0;
  }

  //Verification argv[2] is a number
  for(int i = 0; i < strlen(argv[2]); i++){
    if(!isdigit(argv[2][i])){
      cout << "Error: El numero de paquetes debe ser un numero." << endl;
      cout << "Uso: ./pingIpThreads archivo.txt numerodepaquetes"<< endl;
      return 0;
    }
  }

  //Verification argv[1] is a txt file
  string fileName = argv[1];
  if(fileName.substr(fileName.find_last_of(".") + 1) != "txt"){
    cout << "Error: El archivo de ips debe ser un archivo .txt." << endl;
    return 0;
  }
  
  //Get the number of lines from .txt file in argv[1] and store it.
  int numLines = 0;
  string line;
  ifstream txtFile (argv[1]);
  if (!txtFile.is_open()) {
    cout << "Error: No se ha podido abrir el archivo" << endl;
    return 1;
  }
  while (getline(txtFile, line)) {
    numLines++;
  }

  //Create an array of strings with the lines of the .txt file. Called ipList.
  string ipList[numLines];

  //Read the .txt file again and store the lines in the array.
  txtFile.clear();
  txtFile.seekg(0, ios::beg);
  int i = 0;
  while (getline(txtFile, line)) {
    ipList[i] = line;
    i++;
  }

  //Create an array of threads with the number of lines in the file.
  thread threads[numLines];
  //Create an array of ipData with the number of lines in the file.
  ipData ipDataList[numLines];

  //Create a thread for each line in the file.
  for (i=0; i < numLines; i++) {
    threads[i] = thread(pingIp, ipList[i], argv[2], &ipDataList[i]);
  }

  //Join all threads.
  for (i=0; i < numLines; i++) {
    threads[i].join();
  }
  //Print the data of each ip from ipDataList.

  cout << "IP\t\t\tTrans.\tRec.\tLost\tStatus" << endl;
  cout << "--------------------------------------------------------" << endl;
  for (i=0; i < numLines; i++) {
    if(ipDataList[i].ip.length() >= 16)
      cout << ipDataList[i].ip << "\t" << ipDataList[i].packetsTransmitted << "\t" << ipDataList[i].packetsReceived << "\t" << ipDataList[i].packetsLost << "\t" << ipDataList[i].status << endl;
    else if (ipDataList[i].ip.length() >= 8)
      cout << ipDataList[i].ip << "\t\t" << ipDataList[i].packetsTransmitted << "\t" << ipDataList[i].packetsReceived << "\t" << ipDataList[i].packetsLost << "\t" << ipDataList[i].status << endl;
    else 
      cout << ipDataList[i].ip << "\t\t\t" << ipDataList[i].packetsTransmitted << "\t" << ipDataList[i].packetsReceived << "\t" << ipDataList[i].packetsLost << "\t" << ipDataList[i].status << endl;
  }
  txtFile.close();

  return 0;
}

