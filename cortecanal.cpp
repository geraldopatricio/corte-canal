#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include <MySQL_Encrypt_Sha1.h>
#include <MySQL_Packet.h>
#include <Ethernet.h>

// MAC Address caso queria deixa amarrado no dhcp esse é o mac, pode ser mudado
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; 

// led integrado na placa - pino D13 
#define LED_D13 13                   

IPAddress server_ip(xxx,xx,0,3); // ip do servidor de banco mysql
IPAddress ip(xxx,xx,0,144);  // qualquer ip da faixa de rede para caso o arduino nao suba dhcp
EthernetClient client;

char user[] = "USER"; //user banco
char password[] = "SENHA"; //senha banco
MySQL_Connection conn((Client *)&client);

const byte placa1 = 2; // pin da referida placa
unsigned long placa1_counter; // apenas instancia pra ser atualizado depois

void setup() {
  Serial.begin(9600);
  pinMode(placa1, INPUT_PULLUP);
  pinMode(LED_D13, OUTPUT); // configura pino Led_D13 como saída
  if (Ethernet.begin(mac) == 0) {
  Serial.println("Falha ao configurar Ethernet usando DHCP");
  Ethernet.begin(mac, ip);
  }else{
    delay(1000);
    Serial.println("comecem os trabalhos");
    if (conn.connect(server_ip, 3306, user, password)) {
      delay(1000);
      Serial.println("Connection ok.");
      carrega_counters();
    }else{
      Serial.println("Falha na Conexão.");
    }
    //conn.close();
  }
}

void carrega_counters(){
  MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
  cur_mem->execute("select counter from arduino_fae_apt.arduino_contadores where pin = 2");
  column_names *columns = cur_mem->get_columns();
  row_values *row = NULL;
  do {
    row = cur_mem->get_next_row();
    if (row != NULL) {
     placa1_counter = atol(row->values[0]);    
    }
  } while (row != NULL);
  Serial.print("Placa - Corte Canal 1: ");
  Serial.println(placa1_counter);
  delete cur_mem;
}
 
void loop()
{
  //digitalWrite(LED_D13, HIGH);       // liga o Led 
  //delay(5000);                       // aguarda um segundo 
  //digitalWrite(LED_D13, LOW);        // desliga o Led 
  delay(1000);                         // aguarda um segundo 
  if (digitalRead(placa1) == LOW)
  {
    digitalWrite(LED_D13, HIGH);
    Serial.println("Feito! Alto");
    Serial.println("Gravando dados");
    // Inicie a instância da classe de consulta
    MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
    // Executar sql
    char INSERT_DATA[] = "insert into arduino_fae_apt.arduino_registros values (null, 2, %lu, now())";
    char query[128];
    sprintf(query, INSERT_DATA, placa1_counter);
    cur_mem->execute(query);
    placa1_counter++;
    // uma vez que não há resultados, não precisamos ler nenhum dado
    // Excluir o cursor também libera memória usada
    delete cur_mem;
    Serial.println("Gravando...");
  }
  else
  {
    digitalWrite(LED_D13, LOW);
    Serial.println("Feito! Baixo");
  }
}
