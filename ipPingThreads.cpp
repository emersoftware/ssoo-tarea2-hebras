#include <thread>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstring>
using namespace std;
//Emerson Benjamín Salazar Rubilar

//Clase ipData para almacenar los datos de cada ip
class ipData{
public:
    string ip;
    int packetsTransmitted;
    int packetsReceived;
    int packetsLost;
    string status;

    //Constructor de la clase ipData por defecto
    ipData(){
        ip = "";
        packetsTransmitted = 0;
        packetsReceived = 0;
        packetsLost = 0;
        status = "Down";
    }

    //Setters de la clase ipData
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

  //Se crea un puntero a FILE para almacenar el resultado del comando
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
  //Se obtienen los datos de la ip a partir del resultado del comando
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
      packetsLost = stoi(line.substr(line.find("received, ")+10, line.find(" packet loss")-line.find("received, ")+10))/100*packetsTransmitted;
    }
  }
  if(packetsReceived == packetsTransmitted){
    status = "Up";
  }

  //Se almacenan los datos de la ip en un objeto de la clase ipData del arreglo
  ipData->setData(ip, packetsTransmitted, packetsReceived, packetsLost, status);
  
}


int main(int argc, char *argv[]) {
  //Verificación de argumentos de entrada, se debe ingresar el nombre del archivo y el número de paquetes
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

  //Verificacion del tipo de dato del argumento 2, debe ser el numero de paquetes
  for(int i = 0; i < strlen(argv[2]); i++){
    if(!isdigit(argv[2][i])){
      cout << "Error: El numero de paquetes debe ser un numero." << endl;
      cout << "Uso: ./pingIpThreads archivo.txt numerodepaquetes"<< endl;
      return 0;
    }
  }

  //Verificacion del tipo de dato del argumento 1, debe ser el nombre del archivo con extension .txt
  string fileName = argv[1];
  if(fileName.substr(fileName.find_last_of(".") + 1) != "txt"){
    cout << "Error: El archivo de ips debe ser un archivo .txt." << endl;
    return 0;
  }
  
  //Se cuenta el numero de lineas del archivo para saber el numero de ips
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

  //Se crea un arreglo string para almacenar las ips
  string ipList[numLines];

  //Se almacenan las ips en el arreglo
  txtFile.clear();
  txtFile.seekg(0, ios::beg);
  int i = 0;
  while (getline(txtFile, line)) {
    ipList[i] = line;
    i++;
  }

  //Se crea un arreglo de hilos
  thread threads[numLines];
  //Se crea un arreglo de objetos de la clase ipData
  ipData ipDataList[numLines];

  //Se crean los hilos, un hilo por cada ip. Cada hilo ejecuta la funcion pingIp.
  for (i=0; i < numLines; i++) {
    threads[i] = thread(pingIp, ipList[i], argv[2], &ipDataList[i]);
  }

  //Se espera a que terminen los hilos
  for (i=0; i < numLines; i++) {
    threads[i].join();
  }
  //Se cierra el archivo
  txtFile.close();

  //Se imprime la tabla con los datos de las ip.
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

  return 0;
}

