/** 
*   @file     EUROBOT_Init.c
*   @brief    Definicije funkcija za inicijalizaciju periferija.
*   @author   Cuvari plaze(Praetorian)
*   @version  v1.00 (Praetorian)
*   @date     9 June 2016
*/


#include "stm32f10x.h"
#include "stm32f10x_conf.h"
#include "EUROBOT_Init.h"

/**
*   @brief  Inicijalizacija zeljenog pina.
*   @param  GPIOx Port kojem pripada pin koji zelimo da incijalizujemo.
*       @arg GPIOA, GPIOB, GPIOC, GPIOD, GPIOE.
*   @param  GPIO_Pin Redni broj pina u portu koji zelimo da inicijalizujemo.
*       @arg GPIO_Pin_0, GPIO_Pin_1, ..., GPIO_Pin_15.
*   @param  GPIO_Mode Rezim rada zeljenog pina.
*       @arg GPIO_Mode_IN_FLOATING  Pin se ponasa kao ulaz a ako nije povezan, pin lebdi.
*       @arg GPIO_Mode_IPD          Pin se ponasa kao ulaz i vezan je slabim pull-down otpornikom na negativno napajanje.
*       @arg GPIO_Mode_IPU          Pin se ponasa kao ulaz i vezan je slabim pull-up otpornikom na napajanje.
*       @arg GPIO_Mode_AIN          Pin se ponasa kao analgni ulaz, sto znaci da se signal direktno sa ulaza vodi dalje u mikrokontroler
*                                   (nema smestanja u prihvatne registre i slicno). Korisno je sve pinove koje ne koristimo postaviti u 
*                                   ovo stanje radi ustede energije.
*       @arg GPIO_Mode_Out_PP       Pin se postavlja kao izlaz ali izlaz iz Output Data Register-a je vezan preko push-pull mreze sa
*                                   spoljnim svetom, '0' u ODR-u aktivira NMOS tranzistor (izlaz vezan na masu) a '1' aktivira PMOS 
*                                   tranzistor (izlaz vezan na napajanje).
*       @arg GPIO_Mode_Out_OD       Pin se ponasa kao izlaz ali izlaz iz Output Data Register-a je vezan preko open-drain mreze sa spoljnim
*                                   svetom, '0' u ODR-u aktivira NMOS tranzistor (izlaz vezan na masu, a '1' u ODR-u stavlja izlaz u stanje
*                                   visoke impedanse (PMOS tranzistor se nikad ne aktivira).
*       @arg GPIO_Mode_AF_PP        Pin se ponasa kao izlaz ali je sada na izlazni bafer vezana neka periferija a ne ODR porta. Ostalo je
*                                   isto kao i kod GPIO_OUT_PP.
*       @arg GPIO_Mode_AF_OD        Pin se ponasa kao izlaz ali je sada na izlazni bafer vezana neka periferija a ne ODR porta. Ostalo je 
*                                   isto kao i kod GPIO_OUT_OD.
*   @param  GPIO_Speed Definise najvecu brzinu kojom moze da se menja signal na pinu. Slew rate.
*       @arg GPIO_Speed_10MHz, GPIO_Speed_2MHz, GPIO_Speed_50MHz.
*   @note   Obicno nam je potrebna sto veca brzina pina pa ce za nas default biti GPIO_Speed_50Mhz.
*
*   Ova funkcija se koristi za inicijalizaciju bilo kog pina na mikrokontroleru. Obratiti paznju da je potrebno inicijalizovati pin i ako ga 
*   koristimo u Alternate Function rezimu. Tada se bira jedna od Alternate Function opcija za rezim rada. Alternate Function se koristi kad god
*   koristimo neku drugu periferiju ciji se koriscen izlaz na mikrokontroleru poklapa sa nekim od pinova portova.
*/
void InitGPIO_Pin( GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, GPIOMode_TypeDef GPIO_Mode, GPIOSpeed_TypeDef GPIO_Speed )
{
  /* Dovodjenje CLK signala do porta GPIOx. */
  if ( GPIOx == GPIOA )      RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA, ENABLE );
  else if ( GPIOx == GPIOB ) RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB, ENABLE );
  else if ( GPIOx == GPIOC ) RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC, ENABLE );
  else if ( GPIOx == GPIOD ) RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOD, ENABLE );
  else if ( GPIOx == GPIOE ) RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOE, ENABLE );
  
  /* Potrebno ako nam je pin u alternate function rezimu. */
  if ( GPIO_Mode == GPIO_Mode_AF_PP || GPIO_Mode == GPIO_Mode_AF_OD ) RCC_APB2PeriphClockCmd( RCC_APB2Periph_AFIO, ENABLE );
  
  /* Inicijalizacija strukture koja sluzi sa konfigurisanje zeljenog pina. */
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_StructInit( &GPIO_InitStructure );
  
  /* Postavljanje rednog broja pina koji se konfigurise. */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin;
  
  /* Postavljanje rezima rada pina koji se konfigurise. */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode;
  
  /* Postavljanje brzine pina koji se konfigurise. */
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed;
  
  /* Konfigurisanje pina. */
  GPIO_Init( GPIOx, &GPIO_InitStructure );
}
/******************************************************************************************************************************************************/


/**
*   @brief  Inicijalizuje se Time Base tajmera. Definisu se preskaler, nacin brojanja i vrednost do koje se broji.
*   
*   @param  TIMx  Tajmer koji se inicijalizuje.
*       @arg TIM1, TIM2, TIM3, TIM4, TIM5, TIM15, TIM16, TIM17.
*   @note   Ova funkcija se ne moze koristiti za inicijalizaciju TIM6 i TIM7.
*   @param  TIM_Prescaler  Na koliko uzlaznih ivica ulaznog signala takta brojac promeni vrednost.
*       @arg uint16_t, 0x0000 - 0xFFFF.
*   @note   Obratiti paznju da nula oznacava samo da se broji na svaku uzlaznu ivicu, tako da ako preskalirate sa
*           vrednoscu x, kao argument dajte vrednost x-1.
*   @param  TIM_Period  Vrednost posle koje se brojac resetuje. Generise se Update Event (UEV).
*       @arg uint16_t, 0x0000 - 0xFFFF.
*   @note   Kombinacijom parametara prescaler i period se definise zeljena ucestalost. Obratiti paznju da ako zelite
            da vam broji x puta unesite vrednost x-1 jer brojanje pocinje od nule.
*   @param  TIM_CounterMode  Rezim brojanja. 
*       @arg TIM_CounterMode_Up             Brojac broji navise.
*       @arg TIM_CounterMode_Down           Brojac broji nanize.
*       @arg TIM_CounterMode_CenterAlignedX Brojac broji navise do vrednosti TIM_Period - 1, generise UEV pa onda broji
*                                           nanize do 0 + 1, generise UEV i resetuje se. X moze biti 1, tad se generise
*                                           UEV samo kad brojac broji nadole, moze biti 2, tad se generise UEV samo kad
*                                           brojac broji nagore, i moze biti 3, tad se generise UEV u oba slucaja.
*   @note   TREBA TESTIRATI CenterAligned!!!!
*   @param  TIM_ClockDivision Odnos frekvencije internog signala takta i ucestalosti sempliranja digitalnih filtara na
*           ulazu tajmera. Ovi ulazi se koriste za Input Capture.
*       @arg TIM_CKD_DIV1, TIM_CKD_DIV2, TIM_CKD_DIV4.
*   @note   Kako uglavnom necemo koristiti filtere, ovaj parametar ce uvek biti TIM_CKD_DIV1.
*   @param  TIM_RepetitionCounter Dodatni brojac u nekim tajmerima koji moze da broji samo na dole i dekrementira se kad 
*           god glavni tajmer aktivira Overflow/Underflow. UEV se generise samo kad Repetition Counter dosegne nulu a ne
*			kad glavni tajmer aktivira Overflow/Underflow.
*       @arg uint8_t, 0x00 - 0xFF.
*   @note   Repetition Timer imaju tajmeri TIM1, TIM15, TIM16, TIM17.
*   @see    InitGPIOPin, InitTIM_OC, InitTIM_IC, InitNVICChannel
* 
*   Ova funkcija sluzi za konfigurisanje vremenske baze tajmera. Ova funkcija se obicno ne koristi samostalno. U slucaju
*   da tajmer koristi jedan od kanala, pored ove funkcije se moraju koristiti funkcija InitGPIO_Pin za konfigurisanje pina
*   na koji je odgovarajuci ulaz/izlaz tajmera vezan, zatim InitTIM_OC ili InitTIM_IC da konfigurise zeljene kanale tajmera i
*   eventualno InitNVIC_Channel za dozvolu prekida i promenu prioriteta prekida koje tajmer moze da generise.
*/
void InitTIM_TimeBase( TIM_TypeDef *TIMx, uint16_t TIM_Prescaler, uint16_t TIM_Period, uint16_t TIM_CounterMode, uint16_t TIM_ClockDivision, uint8_t TIM_RepetitionCounter )
{
  /* Dovodjenje signala takta do zeljenog tajmera. */
  if ( TIMx == TIM1 )       RCC_APB2PeriphClockCmd( RCC_APB2Periph_TIM1, ENABLE );
  else if ( TIMx == TIM2 )  RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM2, ENABLE );
  else if ( TIMx == TIM3 )  RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM3, ENABLE );
  else if ( TIMx == TIM4 )  RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM4, ENABLE );
  else if ( TIMx == TIM5 )  RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM5, ENABLE );
  else if ( TIMx == TIM8 )  RCC_APB2PeriphClockCmd( RCC_APB2Periph_TIM8, ENABLE );
  else if ( TIMx == TIM15 ) RCC_APB2PeriphClockCmd( RCC_APB2Periph_TIM15, ENABLE );
  else if ( TIMx == TIM16 ) RCC_APB2PeriphClockCmd( RCC_APB2Periph_TIM16, ENABLE );
  else if ( TIMx == TIM17 ) RCC_APB2PeriphClockCmd( RCC_APB2Periph_TIM17, ENABLE );
  
  /* Inicijalizacija strukture za konfigurisanje tajmera. */
  TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
  TIM_TimeBaseStructInit( &TIM_TimeBaseInitStructure );
  
  /* Konfigurisanje vrednosti preskalera. */
  TIM_TimeBaseInitStructure.TIM_Prescaler = TIM_Prescaler;
  
  /* Konfigurisanje vrednosti do koje ili od koje glavni brojac broji. */
  TIM_TimeBaseInitStructure.TIM_Period = TIM_Period;
  
  /* Konfigurisanje rezima brojanja tajmera.*/
  TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode;
  
  /* Konfigurisanje ucestalosti sempliranja digitalnih filtara na ulazu tajmera. */
  TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_ClockDivision;
  
  /* Konfigurisanje vrednosti u pomocnom Repetition brojacu. */
  TIM_TimeBaseInitStructure.TIM_RepetitionCounter = TIM_RepetitionCounter;
  
  TIM_TimeBaseInit( TIMx, &TIM_TimeBaseInitStructure );
  
  /* Pokretanje tajmera. */
  TIM_Cmd( TIMx, ENABLE );
}
/******************************************************************************************************************************************************/


/**
*   @brief  Inicijalizuje kanal tajmera u output compare modu.
*   @param: TIMx pokazivac na zeljeni tajmer koji sadrzi adresu odgovarajuce
*           periferije.
*           Moguce vrednosti su: TIM1, TIM2, ..., TIM17.
*   @param: channel predstavlja redni broj kanala.
*           Moguce vrednosti:
*              @arg: TIM_Channel_1, selektuje prvi kanal.
*              @arg: TIM_Channel_2, selektuje drugi kanal.
*              @arg: TIM_Channel_3, selektuje treci kanal.
*              @arg: TIM_Channel_4, selektuje cetvrti kanal.
*   @param: output_state otvara ili zatvara odgovarajuci kanal.
*           Moguce vrednosti:
*              @arg: TIM_OutputState_Enable, otvara kanal.
*              @arg: TIM_OutputState_Disable, zatvara kanal.
*   @param: oc_mode definise tip signala koji se generise na izlazu kanala.
*           Moguce vrednosti:
*              @arg TIM_OCMode_Timing,   poredjenje sadrzaja registara CCRx i CNT nemaju
*                                         uticaja na vrednost na izlazu.
*              @arg TIM_OCMode_Active,   izlaz se stavlja na 1 kad se sadrzaji registara
*                                         CCRx i CNT poklope.
*              @arg TIM_OCMode_Inactive, izlaz se stavlja na nulu kad se sadrzaji
*                                         registara CCRx i CNT poklope.
*              @arg TIM_OCMode_Toggle,   kad se poklope sadrzaji registara CCRx i CNT
*                                         vrednost na izlazu se promeni.
*              @arg TIM_OCMode_PWM1,     generise impulse koji su edge-aligned.
*              @arg TIM_OCMode_PWM2,     generise impulse koji su center-aligned.
*   @param  pulse Definise vrednost u CCR-u kanala. Moguce vrednosti su od 0x0000 do 0xFFFF.
*   @param  polarity Definise da li je izlaz aktivan na logickoj jedinici ili logickoj nuli.
*       @arg TIM_OCPolarity_High  Postavlja aktivan nivo na logicku jedinicu.
*       @arg TIM_OCPolarity_Low   Postavlja aktivan nivo na logicku nulu.
*
*/
void InitTIM_OC(TIM_TypeDef *TIMx, uint16_t channel, uint16_t output_state, uint16_t oc_mode, uint16_t pulse, uint16_t polarity)
{
    // Inicijalizacija kanala.
    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_OCStructInit( &TIM_OCInitStructure );
	
    // Postavlja bit 0 u TIMx_CCER.
    TIM_OCInitStructure.TIM_OutputState = output_state;
	
    // Postavlja bite 6,5,4 ili 14,13,12 u TIMx_CCMRx.
    TIM_OCInitStructure.TIM_OCMode = oc_mode;
	
    // Upisuje vrednost u CCRx.
    TIM_OCInitStructure.TIM_Pulse = pulse;
    
    // Postavlja bit 1 u TIMx_CCER.
    TIM_OCInitStructure.TIM_OCPolarity = polarity;
	
    // Izbor kanala koji se inicijalizira.
    if (channel == TIM_Channel_1) TIM_OC1Init( TIMx, &TIM_OCInitStructure );
    if (channel == TIM_Channel_2) TIM_OC2Init( TIMx, &TIM_OCInitStructure );
    if (channel == TIM_Channel_3) TIM_OC3Init( TIMx, &TIM_OCInitStructure );
    if (channel == TIM_Channel_4) TIM_OC4Init( TIMx, &TIM_OCInitStructure );

    // Startovanje tajmera.
    TIM_Cmd( TIMx, ENABLE );
}

/**
*   @brief: Inicijalizuje kanal tajmera u input capture modu.
*   @param: TIMx 
*   @param: TIMx pokazivac na zeljeni tajmer koji sadrzi adresu odgovarajuce
*           periferije. Moguce vrednosti su: TIM1, TIM2, ..., TIM17.
*   @param: channel predstavlja redni broj kanala.
*           Moguce vrednosti:
*              @arg: TIM_Channel_1, selektuje prvi kanal.
*              @arg: TIM_Channel_2, selektuje drugi kanal.
*              @arg: TIM_Channel_3, selektuje treci kanal.
*              @arg: TIM_Channel_4, selektuje cetvrti kanal.
*   @param: selection odredjuje da li kanal radi kao ulaz ili kao izlaz. Ako radi kao izlaz onda
*           se navodi i da li je na kanal vezan ulaz TI1 ili TI2 (ako su kanali 1 i 2) odnosno ulazi
*           TI3 i TI4(ako su kanali 3 i 4). Moguce vrednosti su:
*              @arg: TIM_ICSelection_DirectTI,   kanali 1,2,3,4 odgovaraju ulazima TI1,TI2,TI3,TI4.
*              @arg: TIM_ICSelection_IndirectTI, kanali 1,2,3,4 odgovaraju ulazima TI2,TI1, TI4, TI3.
*              @arg: TIM_ICSelection_TRC,        kanal koji se inicijalizuje vezan je na TRC.
*   @param: prescaler definise na koliko dogadjaja se vrsi input capture. Moguce vrednosti su:
*              @arg: TIM_ICPSC_DIV1, input capture se vrsi na svaku ivicu ulaznog signala(na svaki dogadjaj).
*              @arg: TIM_ICPSC_DIV2, input capture se vrsi na dva dogadjaja.
*              @arg: TIM_ICPSC_DIV4, input capture se vrsi na cetiri dogadjaja.
*              @arg: TIM_ICPSC_DIV8, input capture se vrsi na osam dogadjaja.
*   @note:  Obicno cemo po default-u koristiti TIM_ICPSC_DIV1.
*   @param: filter definise ucestalost kojom se semplira ulaz TIx. Moguce vrednosti su celobrojne 
*           velicine od 0x0 do 0xF.
*   @note:  Za nase potrebe default vrednost nam je 0x0.
*   @param: polarity definise na koju vrstu dogadjaja ulaznog signala se vrsi input capture.
*              @arg: TIM_ICPolarity_Rising,   input capture se vrsi na uzlaznu ivicu.
*              @arg: TIM_ICPolarity_Falling,  input capture se vrsi na silaznu ivicu.
*              @arg: TIM_ICPolarity_BothEdge, input capture se vrsi na obe ivice.
*   @return: 
*
*/
void InitTIM_IC(TIM_TypeDef *TIMx, uint16_t channel, uint16_t selection, uint16_t prescaler, uint16_t filter, uint16_t polarity)
{
    // Inicijalizacija kanala.
    TIM_ICInitTypeDef TIM_ICInitStructure;
    TIM_ICStructInit( &TIM_ICInitStructure );
	
    // Postavljanje odgovarajuceg kanala.
    TIM_ICInitStructure.TIM_Channel = channel; 
    
    // Postavlja bite 9,8 ili 1,0 u CCMRx.
    TIM_ICInitStructure.TIM_ICSelection = selection;
	
    // Postavlja bite 11,10 ili 3,2 u CCMRx.
    TIM_ICInitStructure.TIM_ICPrescaler = prescaler;
	
    // Postavlja bite 15,14,13,12 ili 7,6,5,4 u CCMRx.
    TIM_ICInitStructure.TIM_ICFilter = filter;
	
    // Postavlja bit 1 CCER.
    TIM_ICInitStructure.TIM_ICPolarity = polarity;
	
    
    TIM_ICInit(TIMx, &TIM_ICInitStructure );
    // Startovanje tajmera.
    TIM_Cmd( TIMx, ENABLE );
}

/**
*   @brief: Inicijalizacija jednog kanala periferije NVIC (periferija koja prima zahteve za prekid).
*   @param: NVIC_Channel predstavlja ime kanala koji zelimo da inicijalizujemo, svi kanali su vezani za
*           odgovarajuce periferije. Cela lista mogucih vrednosti se moze videti u fajlu stm32f10x.h na 
*           liniji 144 i 316, ovde su navedene samo one koje se najcesce koriste:
*              @arg: TIM2_IRQn,   prekid od strane tajmera 2.
*              @arg: EXTI0_IRQn,  eksterni prekid na liniji 0.
*              @arg: USART1_IRQn, prekid od UART-a 1.
*   @param: NVIC_PreemptionPriority definise prioritet kanala, broj prioriteta i podprioriteta se definise sa
*           istom grupom od 4 bita, pa ako se sva 4 bita koriste za prioritete znaci da imamo 16 prioriteta
*           i 0 podprioriteta, ako recimo koristimo 2 bita za prioritete znaci da imamo 4 prioriteta i svaki
*           prioritet ima 4 podprioriteta. Nizi prioritet ima prednost. Ako tokom izvrsavanja prekidne rutine 
*           stigne zahtev za prekid sa nizim prioritetom odmah se skace na opsluzivanje novog zahteva za prekid.
*           Ovo nije slucaj kod podprioriteta.
*           Moguce vrednosti su celobrojne vrednosti iz opsega 0 do 2^brojBitaZaPrioritet -1.
*   @note:  Pozivom funkcije void NVIC_PriorityGroupConfig(uint32_t NVIC_PriorityGroup) u glavnom programu se odredjuje
*           koliko bita se koristi za prioritete. Moguci argumenti su: NVIC_PriorityGroup_0 , NVIC_PriorityGroup_1,
*           NVIC_PriorityGroup_2, NVIC_PriorityGroup_3, NVIC_PriorityGroup_4.
*   @param: NVIC_SubPriority definise podprioritet kanala.
*           Moguce vrednosti su sve celobrojne vrednosti iz opsega 0 do 2^(4-brojBitaZaPrioritet) - 1.
*   @param: NVIC_ChannelCmd odredjuje da li NVIC otvara ovaj kanal i ne.
*           Moguce vrednosti su ENABLE i DISABLE.
*   @return: Nema povratne vrednosti.
*
*/
void InitNVICChannel(uint8_t NVIC_Channel, uint8_t NVIC_PreemptionPriority, uint8_t NVIC_SubPriority, FunctionalState NVIC_ChannelCmd)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  
  NVIC_InitStructure.NVIC_IRQChannel = NVIC_Channel;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = NVIC_PreemptionPriority;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = NVIC_SubPriority;
  NVIC_InitStructure.NVIC_IRQChannelCmd = NVIC_ChannelCmd;
  
  NVIC_Init(&NVIC_InitStructure);
}

/**
*   @brief: Inicijalizuje eksternu liniju prekida.
*   @param: GPIO_PortSourceX definise na kojem portu je prikljucen eksterni prekid.
*           Moguce vrednosti su: GPIO_PortSourceGPIOA, ..., GPIO_PortSourceGPIOG.
*   @param: Line definise koja se eksterna linija prekida inicijalizuje.
*           Moguce vrednosti su: EXTI_Line0, ..., EXTI_Line15.
*   @param: Mode definise rezim u kome ce linija raditi.
*           Moguce vrednosti su:
*              @arg: EXTI_Mode_Interrupt,
*              @arg: EXTI_Mode_Event,
*   @param: Trigger definise sta su okidaci za eksternu liniju.
*           Moguce vrednosti su:
*              @arg: EXTI_Trigger_Rising,         uzlazna ivica.
*              @arg: EXTI_Trigger_Falling,        silazna ivica.
*              @arg: EXTI_Trigger_Rising_Falling, uzlazna i silazna ivica.
*   @param: LineCmd definise da li se linija aktivira ili ne.
*           Moguce vrednosti su ENABLE i DISABLE.
*   @return: Nema povratne vrednosti.
*
*/
void InitEXTI(uint8_t GPIO_PortSourceX, uint32_t Line, EXTIMode_TypeDef Mode, EXTITrigger_TypeDef Trigger, FunctionalState LineCmd)
{
  //Konfigurisanje odgovarajuceg pina na zeljenom portu kao eksterni prekid.
  if (Line == EXTI_Line0) GPIO_EXTILineConfig(GPIO_PortSourceX, GPIO_PinSource0);
  if (Line == EXTI_Line1) GPIO_EXTILineConfig(GPIO_PortSourceX, GPIO_PinSource1);
  if (Line == EXTI_Line2) GPIO_EXTILineConfig(GPIO_PortSourceX, GPIO_PinSource2);
  if (Line == EXTI_Line3) GPIO_EXTILineConfig(GPIO_PortSourceX, GPIO_PinSource3);
  if (Line == EXTI_Line4) GPIO_EXTILineConfig(GPIO_PortSourceX, GPIO_PinSource4);
  if (Line == EXTI_Line5) GPIO_EXTILineConfig(GPIO_PortSourceX, GPIO_PinSource5);
  if (Line == EXTI_Line6) GPIO_EXTILineConfig(GPIO_PortSourceX, GPIO_PinSource6);
  if (Line == EXTI_Line7) GPIO_EXTILineConfig(GPIO_PortSourceX, GPIO_PinSource7);
  if (Line == EXTI_Line8) GPIO_EXTILineConfig(GPIO_PortSourceX, GPIO_PinSource8);
  if (Line == EXTI_Line9) GPIO_EXTILineConfig(GPIO_PortSourceX, GPIO_PinSource9);
  if (Line == EXTI_Line10) GPIO_EXTILineConfig(GPIO_PortSourceX, GPIO_PinSource10);
  if (Line == EXTI_Line11) GPIO_EXTILineConfig(GPIO_PortSourceX, GPIO_PinSource11);
  if (Line == EXTI_Line12) GPIO_EXTILineConfig(GPIO_PortSourceX, GPIO_PinSource12);
  if (Line == EXTI_Line13) GPIO_EXTILineConfig(GPIO_PortSourceX, GPIO_PinSource13);
  if (Line == EXTI_Line14) GPIO_EXTILineConfig(GPIO_PortSourceX, GPIO_PinSource14);
  if (Line == EXTI_Line15) GPIO_EXTILineConfig(GPIO_PortSourceX, GPIO_PinSource15);
  
  //Inicijalizacija eksternog prekida.
  EXTI_InitTypeDef EXTI_InitStructure;
  EXTI_InitStructure.EXTI_Line = Line;
  EXTI_InitStructure.EXTI_Mode = Mode;
  EXTI_InitStructure.EXTI_Trigger = Trigger; 
  EXTI_InitStructure.EXTI_LineCmd = LineCmd;
  EXTI_Init(&EXTI_InitStructure);
}

/**
*   @brief: Incijializacija USART periferije.
*   @param: USARTx pokazivac na USART periferiju koju zelimo da inicijializujemo.
*           Moguce vrednosti su: USART1, USART2, USART3.
*   @param: Mode definise rezim u kome ce USART raditi.
*           Moguce vrednosti su:
*              @arg: USART_Mode_Tx, salje podatke duz linije Tx.
*              @arg: USART_Mode_Rx, prima podatke duz linije Rx.
*   @param: BaudRate definise ucestalost na kojoj radi USART.
*   @param: WordLength definise broj bita koji se salju ili primaju u jednom frejmu.
*           Moguce vrednosti:
*              @arg: USART_WordLength_8b, broj bita koji nose poruku je 8.
*              @arg: USART_WordLength_9b, broj bita koji nose poruku je 9.
*   @param: StopBits definise koliko se stop bita salje.
*           Moguce vrednosti su: 
*              @arg: USART_StopBits_1, salje se 1 stop bit.
*              @arg: USART_StopBits_2, salje se 2 stop bita.
*   @param: Parity definise da li se u frejmu salje i bit parnosti i ako da, da li je
*           paran ili ne.
*           Moguce vrednosti:
*              @arg: USART_Parity_No,   ne salje se bit za proveru parnosti.
*              @arg: USART_Parity_Even, salje se bit parnosti i proverava se da li paran broj jedinica.
*              @arg: USART_Parity_Odd,  salje se bit parnosti i proverava se da li je neparan broj jedinica.
*   @return: Nema povratne vrednosti.
*
*/
void InitUSART(USART_TypeDef* USARTx, uint16_t Mode, uint32_t BaudRate, uint16_t WordLength, uint16_t StopBits, uint16_t Parity)
{
  //Dovodjenje clock signala do odgovarajuce USART periferije i inicijalizacija odgovarajucih pinova.
  if (USARTx == USART1)
  {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    InitGPIOPin(GPIOA, GPIO_Pin_9, GPIO_Mode_AF_PP, GPIO_Speed_50MHz);          //Linija Tx za USART1.
    InitGPIOPin(GPIOA, GPIO_Pin_10, GPIO_Mode_IN_FLOATING, GPIO_Speed_50MHz);   //Linija Rx za USART1.
  }
  if (USARTx == USART2)
  {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    InitGPIOPin(GPIOA, GPIO_Pin_2, GPIO_Mode_AF_PP, GPIO_Speed_50MHz);          //Linija Tx za USART2.
    InitGPIOPin(GPIOA, GPIO_Pin_3, GPIO_Mode_IN_FLOATING, GPIO_Speed_50MHz);    //Linija Rx za USART2.
  }
  if (USARTx == USART3)
  {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    InitGPIOPin(GPIOB, GPIO_Pin_10, GPIO_Mode_AF_PP, GPIO_Speed_50MHz);         //Linija Tx za USART3.
    InitGPIOPin(GPIOB, GPIO_Pin_11, GPIO_Mode_IN_FLOATING, GPIO_Speed_50MHz);   //Linija Rx za USART3.
  }
  
  //Inicijalizacija USARTx periferije.
  USART_InitTypeDef USART_InitStructure;
  USART_StructInit(&USART_InitStructure);
  USART_InitStructure.USART_BaudRate = BaudRate;
  USART_InitStructure.USART_WordLength = WordLength;
  USART_InitStructure.USART_StopBits = StopBits;
  USART_InitStructure.USART_Parity = Parity;
  USART_InitStructure.USART_Mode = Mode;
  USART_Init(USARTx ,&USART_InitStructure);
  
  //Paljenje periferije.
  USART_Cmd(USARTx , ENABLE);
}

/**
*   @brief: Incijializacija USART periferije sa default vrednostima. Ovo podrazumeva
*           da je duzina reci po frejmu 8, da imamo jedan stop bit i da nema bita za
*           proveru parnosti.
*   @param: USARTx pokazivac na USART periferiju koju zelimo da inicijializujemo.
*           Moguce vrednosti su: USART1, USART2, USART3.
*   @param: Mode definise rezim u kome ce USART raditi.
*           Moguce vrednosti su:
*              @arg: USART_Mode_Tx, salje podatke duz linije Tx.
*              @arg: USART_Mode_Rx, prima podatke duz linije Rx.
*   @param: BaudRate definise ucestalost na kojoj radi USART.
*   @return: Nema povratne vrednosti.
*
*/
void InitDefaultUSART(USART_TypeDef* USARTx, uint16_t Mode, uint32_t BaudRate)
{
  InitUSART(USARTx, Mode, BaudRate, USART_WordLength_8b, USART_StopBits_1, USART_Parity_No);
}
