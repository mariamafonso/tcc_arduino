/************************************************************************
*   Código para navegação de um robô baseado em Arduino Mega            *
*   através de um controlador nebuloso.                                 *
*                                                                       *
*   Trabalho de Conclusão de Curso                                      *
*   Mariam Afonso (mariam.afonso@gmail.com)                             *
*   Universidade Federal do Rio de Janeiro - UFRJ                       *
*   Departamento de Ciência da Computação - DCC                         *
*   Laboratório de Inteligência Computacional - LabIC                   *
*   Julho de 2012                                                       *
*                                                                       *
*   (Colaboração de Mário Cecchi.)                                      *
************************************************************************/


#include <Servo.h>
#include <math.h>
#include "ponto.h"
#include "libFuzzy.h"


//Número do passo atual:
int passos = 0;

////LibPosicao\\\\
//Valor de referência para a horizontal do ambiente:
float horizontal;
//Ângulo da frente do robô com a horizontal:
float phi;
//Dados auxiliares da iteração anterior:
float prevPhi = 0;
float prevGamma = 0;

////LibRodas\\\\
//Velocidades das rodas:
const int velDir = 20;
const int velEsq = 13;
//Índices de linha da tabela de giro (à direita e à esquerda):
const int giroDir = 0;
const int giroEsq = 1;
//Sentido do giro:
int sentidoGiro;

//Fator de proporção para medidas de distância entre
//os mundos virtual (do controlador) e real (6:13.6):
const float fatorPropDist = 2.26;

//Vetores para as entradas e as saídas do controlador:
float entrada[NUMINPUTS];
float saida[NUMOUTPUTS];

//Saídas do controlador: ângulo de giro e tamanho do passo:
float theta;
float delta;


void setup(){
  Serial.begin(115200);
  Serial1.begin(115200);
  Serial2.begin(115200);
  Serial3.begin(115200);
  
  servoPingSetup();
  
  Serial.println("Log de navegacao:");
  Serial.println();
  
  Serial.println("Inicializando o sensor HM55B...");
  Serial.println();
  HM55B_Reset();
//  HM55B_Calibration();
  HM55B_CurrentCalibrationValues();
  
//  setHorizontal();
  currentHorizontal();
  
  //Espera para colocar o robô na posição inicial do ambiente:
  //delay(60000);
  
  //Zerando as saídas do controlador para a 1ª iteração:
  for(int i = 0; i < NUMOUTPUTS; i++){
    saida[i] = 0.0;
  }
  
  //Já precisam estar definidos no início da 1ª iteração:
  theta = saida[0];
  delta = saida[1];
  
//  randomSeed(analogRead(0)); //Usado pra testar a memória
  
  delay(1500);
}


void loop(){
//  debug();
//  delay(600000);
  
  ////////////////////////////////////////////////////////////
  //  Vai ter um limite de passos? Acho interessante,       //
  //  pra não ter que sair correndo atrás do robô, rsrs...  //
  //  Ver o que o Tiago pôs no código dele!                 //
  ////////////////////////////////////////////////////////////
  
  int duracaoMovimento;
  
  while(1){
    //Quantidade de passos:
    passos++;
    Serial.print("--> Passo ");
    Serial.println(passos);
    Serial.println();
    
    //Saídas do controlador na última iteração:
//    theta = saida[0];
//    delta = saida[1];
    Serial.print("Theta = ");
    Serial.println(theta);
    Serial.print("Delta = ");
    Serial.println(delta);
    Serial.println();
    
    //Determina localização:
    phi = getPhi();
    Serial.print("Phi = ");
    Serial.println(phi);
    Serial.println();
    
    //Detecta obstáculos:
    Serial.println("Deteccoes:");
    detectar();
    
    //Chama o pré-processador...
    //1. Atualiza a memória:
    atualizarMemoria();
    
    //2. Define as entradas do controlador:
    definirEntradasControlador();
    
    //Chama o controlador, com saídas theta e delta:
    rodarMeuFis(entrada, saida);
    theta = saida[0];
    delta = saida[1];
    Serial.println();
    Serial.println("Saidas do controlador:");
    Serial.print("   Theta = ");
    Serial.println(theta);
    Serial.print("   Delta = ");
    Serial.print(delta);
    //Correção pelo fator de proporção:
    delta *= fatorPropDist;
    Serial.print("  ==>  ");
    Serial.println(delta);
    
    //Movimentação:
    //1. Girar:
    duracaoMovimento = duracaoGiro();
    Serial.print("   Duracao do giro = ");
    Serial.println(duracaoMovimento);
    if(sentidoGiro == giroDir){
      movimentar(-velDir, velEsq, duracaoMovimento);
    }
    else{
      movimentar(velDir, -velEsq, duracaoMovimento);
    }
    delay(500);
    //2. Andar:
    duracaoMovimento = duracaoPasso();
    Serial.print("   Duracao do passo = ");
    Serial.println(duracaoMovimento);
    movimentar(velDir, velEsq, duracaoMovimento);
    
    prevPhi = phi;
    
    delay(1500);
    
    Serial.println();
    Serial.println("===========================================");
    Serial.println();
  }
}

