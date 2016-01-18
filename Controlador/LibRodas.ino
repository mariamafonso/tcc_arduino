/************************************************************************
*                 Funções para movimentação do robô                     *
*                (interação com os servos das rodas).                   *
*                                                                       *
*    Mariam Afonso (mariam.afonso@gmail.com) - 2012 - LabIC - UFRJ      *
*************************************************************************
*************************************************************************
*   -> Pode-se variar a velocidade dos servos de 0º a 180º, sendo       *
*     esses extremos os valores máximos em sentidos diferentes.         *
*   -> O servo deveria ficar parado em 90º, mas nem sempre funciona.    *
*                                                                       *
*   Roda direita:   |--- avança ---|-- retrocede -|                     *
*     Rotação:     max            min            max                    *
*                                                                       *
*   Velocidade      |--------------|--------------|                     *
*    do servo:      0              90            180                    *
*                                                                       *
*   Roda esquerda:  |-- retrocede -|--- avança ---|                     *
*     Rotação:     max            min            max                    *
*                                                                       *
*                                                                       *
*   -> Neste código, as velocidades das rodas são dadas em              *
*   função da distância a 90 (valor mínimo comum para ambas).           *
*   Ou seja, para qualquer roda, o intervalo de valores                 *
*   possíveis para velocidades é [-90, 90], onde 0 é o mínimo.          *
*                                                                       *
*      Velocidade      |-- retrocede -|--- avança ---|                  *
*      das rodas:     -90             0             90                  *
*                                                                       *
*                                                                       *
*   -> tabelaGiro: tabela de giro (gerada em 15-07-2012).               *
*     Matriz contendo o módulo do ângulo médio de giro das rodas        *
*     (em graus)quando o argumento de delay() é                         *
*     duracaoOffsetDefault*i, onde i > 0 é sua coluna em tabelaGiro.    *
*     Sua linha na matriz é determinada pelo sinal de theta:            *
*     theta < 0 (giro à direita) corresponde à 1ª linha e               *
*     theta > 0 (giro à esquerda), à 2ª.                                *
*     Para essa tabela foi necessário usar a duração mínima             *
*     duracaoGiroMin < duracaoOffsetDefault (i = 0) para obter uma      *
*     interpolação melhor para valores absolutos de theta pequenos.     *
*                                                                       *
*   -> tabelaPasso: tabela de passo (gerada em 10-07-2012).             *
*     Vetor contendo a distância média percorrida pelo robô (em cm)     *
*     quando o argumento de delay() é duracaoOffsetDefault*(i+1),       *
*     onde i é sua posição em tabelaPasso.                              *
************************************************************************/


//Servos das rodas:
Servo rodaDir, rodaEsq;
//Pinos dos servos:
const int pinoDir = 46;
const int pinoEsq = 21;
//Variáveis para interpolação nas tabelas de giro e de passo:
const int duracaoOffsetDefault = 50;
const int duracaoGiroMin = 25;
const int tamTabGiro = 4;
const float tabelaGiro[2][tamTabGiro] = {
  6.8, 17.4, 23.8, 29.8,
  4.4, 10.2, 18.0, 23.0};
const int tamTabPasso = 8;
const float tabelaPasso[] = {
  1.5, 2.75, 3.75, 4.25, 5.25, 5.75, 6.25, 7.25};


void conectarRodas(){
  /***********************************
  *   Conecta os servos das rodas.   *
  ***********************************/
  
  rodaDir.attach(pinoDir);
  rodaEsq.attach(pinoEsq);
}


void desconectarRodas(){
  /**************************************
  *   Desconecta os servos das rodas.   *
  **************************************/
  
  rodaDir.detach();
  rodaEsq.detach();
}


void movimentar(int veloD, int veloE, int duracao){
  /**************************************************************
  *               Realiza a movimentação do robô.               *
  *   O movimento executado (passo ou giro) é determinado       *
  *   pela combinação de sinais de veloD e veloE (velocidades   *
  *   das rodas direita e esquerda, respectivamente).           *
  **************************************************************/
  
  conectarRodas();
  rodaDir.write(90 - veloD);
  rodaEsq.write(90 + veloE);
  delay((unsigned long)duracao);
  desconectarRodas();
}


int duracaoGiro(){
  /***************************************************
  *         Interpolação na Tabela de Giro.          *
  *   Interpola o valor absoluto de theta para       *
  *   encontrar uma duração apropriada para o        *
  *   movimento das rodas.                           *
  *   Também resolve extrapolação, quando o módulo   *
  *   de theta é maior que o maior valor tabelado.   *
  ***************************************************/
  
  int i;
  int duracao;
  //Índice do elemento tabelado imediatamente menor ou igual a |theta|.
  //Inicializado com o valor máximo que pode receber na busca:
  int index = tamTabGiro - 2;
  //Fator de proporção entre os intervalos:
  float proporcao;
  //Tempos referentes a tabelaGiro[index] e tabelaGiro[index+1]:
  int t1, t2;
  
  Serial.println();
  Serial.println("Dados do giro:");
  
  //Se theta == 0, não precisa interpolar:
  if(theta == 0.0){
    Serial.println("   Theta = 0 --> NAO GIRA!");
    return 0;
  }
  
  //Se o giro é à direita...
  Serial.print("   Giro a ");
  if(theta < 0.0){
    sentidoGiro = giroDir;
    Serial.println("direita (theta < 0).");
  }
  //Se o giro é à esquerda...
  else{
    sentidoGiro = giroEsq;
    Serial.println("esquerda (theta > 0).");
  }
  
  for(i = 0; i < tamTabGiro; i++){
    if(abs(theta) <= tabelaGiro[sentidoGiro][i]){
//      Serial.print("   |Theta| <= tabelaGiro[][");
//      Serial.print(i);
//      Serial.print("] = ");
//      Serial.println(tabelaGiro[sentidoGiro][i]);
      index = i - 1;
      break;
    }
  }
//  Serial.print("   index = ");
//  Serial.println(index);
  
  //Se |theta| <= tabelaGiro[sentidoGiro][0], considera-se que é igual:
  if(index == -1){
    duracao = duracaoGiroMin;
  }
  else{
    proporcao = (abs(theta) - tabelaGiro[sentidoGiro][index]);
    proporcao /= (tabelaGiro[sentidoGiro][index + 1]
                   - tabelaGiro[sentidoGiro][index]);
    //Se |theta| está entre o 1º e o 2º valores, o offset é menor:
    if(index == 0){
      t1 = duracaoGiroMin;
      t2 = duracaoOffsetDefault;
    }
    else{
      t1 = duracaoOffsetDefault*(index);
      t2 = duracaoOffsetDefault*(index+1);
    }
    duracao = t1 + proporcao * (t2 - t1);
  }
  
  return duracao; 
}


int duracaoPasso(){
  /*****************************************************
  *          Interpolação na Tabela de Passo.          *
  *   Interpola o valor de delta para encontrar uma    *
  *   duração apropriada para o movimento das rodas.   *
  *   Também resolve extrapolação, quando delta é      *
  *   maior que o maior valor tabelado.                *
  *****************************************************/
  
  int i;
  int duracao;
  //Índice do elemento tabelado imediatamente menor ou igual a delta.
  //Inicializado com o valor máximo que pode receber na busca:
  int index = tamTabPasso - 2;
  //Fator de proporção entre os intervalos:
  float proporcao;
  //Tempos referentes a tabelaPasso[index] e tabelaPasso[index+1]:
  int t1, t2;
  
  Serial.println();
  Serial.println("Dados do passo:");
  
  //Se delta == 0, não precisa interpolar:
  if(delta == 0.0){
    Serial.println("   Delta = 0 --> NAO ANDA!.");
    return 0;
  }
  
  for(i = 0; i < tamTabPasso; i++){
    if(delta <= tabelaPasso[i]){
//      Serial.print("   --> Delta <= tabelaPasso[");
//      Serial.print(i);
//      Serial.print("] = ");
//      Serial.println(tabelaPasso[i]);
      index = i - 1;
      break;
    }
  }
//  Serial.print("   index = ");
//  Serial.println(index);
  
  //Se delta <= tabelaPasso[0], considera-se que é igual:
  if(index == -1){
    duracao = duracaoOffsetDefault;
  }
  else{
    proporcao = (delta - tabelaPasso[index]);
    proporcao /= (tabelaPasso[index + 1] - tabelaPasso[index]);
    t1 = duracaoOffsetDefault*(index+1);
    t2 = duracaoOffsetDefault*(index+2);
    duracao = t1 + proporcao * (t2 - t1);
  }
  
  return duracao; 
}

