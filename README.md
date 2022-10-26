
# Projeto com ESP32 (WROOM-DA)
Servomecanismo acionado por sensor de chuva + sensores DHT11 (umidade/temp) e de solo

## Pinagem (referente a dados)
Sensor de chuva - D21 (Digital/IN)
Servomotor SG90 - D22 (Digital/OUT)
Sensor de temperatura e umidade (DHT11) - D23 (Digital/IN)
Sensor de umidade do solo - D34 (Analógico/IN)

+ Com a biblioteca <wifi.h> **somente** os pinos [ADC1](https://www.mischianti.org/wp-content/uploads/2021/02/ESP32-wroom-32-pinout-mischianti-high-resolution.png) podem ser usados como leitores analógicos.  <small>Não cometa o  mesmo erro</small>

+ Para o sensor de chuva, foi usado somente a saída digital, e para o sensor de umidade de solo, somente a saída analógica
+ Podem ser trocados editando o código fonte referente a parte de pinagem.

## Funcionalidade (sensores)
+ Os links levam para linhas do código relevantes ao texto.

O sensor de chuva, ligado no pino D21, envia um dado digital que é [armazenado numa variável](https://github.com/fabiokenji919/esp32-chuva/blob/3f8ac834f980f387fd96d49dc1eef54461125b82/esp32_chuva/esp32_chuva.ino#L280) dentro do microcontrolador, com o nível baixo - 0, para sinalizar que está chovendo, e o nivel alto - 1, que não está chovendo. 

Essa variável é usado [num laço de condição (if)](https://github.com/fabiokenji919/esp32-chuva/blob/3f8ac834f980f387fd96d49dc1eef54461125b82/esp32_chuva/esp32_chuva.ino#L250) para acionar o servomotor, no pino D22, que ira girar para 180g se estiver chovendo (chuva_D0 == 0), e ira voltar para 0g se não estiver chovendo (chuva_D0 == 1). 

Para evitar que o servomotor fique "louco", foi [criado uma interrupção por timer](https://github.com/fabiokenji919/esp32-chuva/blob/3f8ac834f980f387fd96d49dc1eef54461125b82/esp32_chuva/esp32_chuva.ino#L91)  que faz com que o movimento do servomotor seja feito somente a cada 2 segundos (ajustável mudando os parâmetros do timer). Essa interrupção faz uma variável se tornar verdadeira a cada 2 segundos, [sendo verificada juntamente](https://github.com/fabiokenji919/esp32-chuva/blob/3f8ac834f980f387fd96d49dc1eef54461125b82/esp32_chuva/esp32_chuva.ino#L251) com a variável da chuva, se tornando falsa ao servomotor finalizar o movimento.

O sensor de temperatura DHT11, no pino D23, [tem seus dados tratados](https://github.com/fabiokenji919/esp32-chuva/blob/3f8ac834f980f387fd96d49dc1eef54461125b82/esp32_chuva/esp32_chuva.ino#L227) usando uma biblioteca dedicada para a interpretação de dados, sendo necessário somente incluir a biblioteca e coletar dados a partir de chamadas de funções da respectiva biblioteca.

Por ultimo, o sensor de umidade de solo, no pino D34, é um sensor analógico que envia o valor de 4095 (2<sup>12</sup>-1) quando detecta que o solo está seco, e aproximadamente 1100 quando molhado - a resoluçao de n = 12 bits faz parte do limite do microcontrolador. Como o dado é recebido de forma inversa e não-normalizado ,[é necessário normalizar e "inverter" novamente o dado](https://github.com/fabiokenji919/esp32-chuva/blob/3f8ac834f980f387fd96d49dc1eef54461125b82/esp32_chuva/esp32_chuva.ino#L151) para que no final seja mostrado, usando o comando [constrain](https://www.arduino.cc/reference/en/language/functions/math/constrain/) ,e em seguida, [map](https://www.arduino.cc/reference/en/language/functions/math/map/).

## Funcionalidade (Servidor Web)
+ [Código referente ao servidor web](https://github.com/fabiokenji919/esp32-chuvachuva/blob/3f8ac834f980f387fd96d49dc1eef54461125b82/esp32_chuva/esp32_chuva.ino#L158)

Foi usado um hotspot (devido as redes terem proxy) para que o próprio ESP atuasse como um roteador, tendo um servidor improvisado.
Usando comandos básicos de html é possivel enviar imprimir linhas de HTML a partir do comando [client.print](https://www.arduino.cc/reference/en/libraries/wifi/client.println/), e a página foi configurada para atualizar sozinha a cada 2 segundos

<center>[figura pendente]</center>
