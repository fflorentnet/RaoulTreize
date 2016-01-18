#include <HT1632.h>
#include <images.h>
#include <font_8x4.h>
#include <lib74HC595.h>
#include <EEPROM.h>


/*
   Configuration de l'ecran LED
*/
#define pinCS1 8
#define pinWR 7
#define pinDATA 6

const unsigned long DELAY_SCROLLING_TEXT = 40;
const unsigned long DELAY_CENTERED_TEXT = 2000;
const unsigned long DELAY_CAPSULE_TEXT = 200;
const unsigned long DELAY_RANDOM_TEXT = 780000;
const unsigned long DELAY_LED_BLINK = 500;
const unsigned long DELAY_LED_CHENILLE = 100;
const unsigned long DELAY_BETWEEN_CAPSULES = 500; //Allez tous niquer vos mères

/*
   Definition des PIN compteur
*/
#define LEDCpt 2
#define PDCapsule 3

  #define pinSER 9
  #define pinRCLK 10
  #define pinSRCLK 11
  #define number_of_74hc595s 2
void getCapsuleText(char* text)
{
  long rand = random(16);
  free(text);
  if(rand == (long)0) strcpy(text, "13 points pour Gryffondor!");
  if(rand == (long)1)   strcpy(text,"13 points pour Serpentard!");
  if(rand == (long)2)   strcpy(text,"13 points pour Poufsouffle!");
  if(rand == (long)3)   strcpy(text,"13 points pour Serdaigle!");
  if(rand == (long)4)   strcpy(text,"Bingo!                   ");
  if(rand == (long)5)   strcpy(text,"Its a kind of magic!");
  if(rand == (long)6)   strcpy(text,"Accio Capsule!");
  if(rand == (long)7)   strcpy(text,"It\'s Leviosaaaaa!");
  if(rand == (long)8)   strcpy(text,"Mechant Dobby ! Mechant !");
  if(rand == (long)9)   strcpy(text,"Pas mal pour un Moldu !");
  if(rand == (long)10)  strcpy(text,"Mefait accompli !");
  if(rand == (long)11)  strcpy(text,"Reprends donc du Polynectar !");
  if(rand == (long)12)  strcpy(text,"Promo 13 ! Promo 13!");
  if(rand == (long)13)  strcpy(text,"Perd pas la main!");
  if(rand == (long)14)  strcpy(text,"Gros poulet!");
  if(rand == (long)15)  strcpy(text,"Plafond, cul sec!"); 
}
void getRandomText(char* text)
{
    long rand = random(7);
    free(text);
  if(rand == (long)0)   strcpy(text, "Buvez a vos 13!");
  if(rand == (long)1)   strcpy(text,"La 13, tu la votes ou tu la quittes.");
  if(rand == (long)2)   strcpy(text,"A l\'aise 13!");
  if(rand == (long)3)   strcpy(text,"On parie que t\'as 13 soif !");
  if(rand == (long)4)   strcpy(text,"On ne chatouille jamais un 13 endormi.");
  if(rand == (long)5)   strcpy(text,"La 13 et toi, c\'est tres etroit");
  if(rand == (long)6)   strcpy(text,"Pere de Sang Mele");
  if(rand == (long)7)   strcpy(text,"Un cidre sur le 5954Q !");
  if(rand == (long)8)   strcpy(text,"Des gaufres! Oh oui, des gaufres!");
}
    
void EEPROMWriteInt(int p_address, int p_value)
{
  byte lowByte = ((p_value >> 0) & 0xFF);
  byte highByte = ((p_value >> 8) & 0xFF);
  EEPROM.write(p_address, lowByte);
  EEPROM.write(p_address + 1, highByte);
}
unsigned int EEPROMReadInt(int p_address)
{
  byte lowByte = EEPROM.read(p_address);
  byte highByte = EEPROM.read(p_address + 1);
  return ((lowByte << 0) & 0xFF) + ((highByte << 8) & 0xFF00);
}

volatile bool state = false;
bool a1ctivated = false;
bool a2ctivated = false;
volatile bool bCapsuleLoop = false;
int i = 0;
int cpt = 0;
char cptC[] = "1000000";

int bCapsule = 0;
bool testPin(int a, bool b)
{
 if (analogRead(a) <= 100 && !b && !bCapsule && !bCapsuleLoop)
 {
   b = true;
   bCapsuleLoop = true;
 }
 else if (analogRead(a) > 100 && b && !bCapsule && !bCapsuleLoop)
 {
   bCapsule++;
   b = false;
 } 
 return b;
}
     
/*
   Differents etats du Raoul
*/
typedef enum {AFFICHECOMPTEUR, AFFICHEPHRASECOMPTEUR, AFFICHEPHRASE, AFFICHEIDLE} stateText;
typedef enum {LEDCHENILLE, LEDBLINK, noLED} stateLed;

stateLed myLedState = noLED;
stateText myTextState = AFFICHECOMPTEUR;



uint8_t wd;
uint8_t it;
uint8_t tLed = 0;
uint8_t ledLoop = 0;
char *text;
bool scrolling = false;

unsigned long startTimeLed = 0;
unsigned long startTimeAfficheur = 0;
unsigned long startTimeRandom = 0;
unsigned long startTimeLastCapsule = 0;

void setup () {
  setupShift();
  randomSeed(analogRead(A1));
  cpt = EEPROMReadInt(0);
  
  HT1632.begin(pinCS1, pinWR, pinDATA);
  pinMode(LEDCpt, OUTPUT);
  pinMode(PDCapsule, INPUT);
  digitalWrite(LEDCpt, LOW);
  text = (char*) malloc(512*sizeof(char));  
  attachInterrupt(digitalPinToInterrupt(PDCapsule), compteurInterrupt, FALLING);
  sprintf(text, "%d", cpt);
  clearRegisters();
  myTextState = AFFICHEPHRASECOMPTEUR; 
  myLedState = LEDCHENILLE;

}
void compteurInterrupt()
{
  bCapsule = true;
}
void compteurInc()
{
  cpt++;
  EEPROMWriteInt(0, cpt);
}
void drawScrollingText()
{
  HT1632.clear();
  HT1632.drawText(text, OUT_SIZE - it, 0, FONT_8X4, FONT_8X4_END, FONT_8X4_HEIGHT);
  HT1632.render();
}

void drawCenteredText()
{
  HT1632.clear();
  HT1632.drawText(text, (OUT_SIZE - wd) / 2, 0, FONT_8X4, FONT_8X4_END, FONT_8X4_HEIGHT);
  HT1632.render();
}
/*
   Afficher un texte
   S'il dépasse de l'écran (en largeur), on le fait défiler
*/
void drawAdaptiveText()
{
  if (scrolling)
    drawScrollingText();
  else
    drawCenteredText();
}
void setScrolling()
{
  it = 0;
  wd = HT1632.getTextWidth(text, FONT_8X4_END, FONT_8X4_HEIGHT);
  scrolling = (wd > OUT_SIZE);
}
void drawShape(const byte img [], uint8_t width, uint8_t height)
{
  HT1632.clear();
  HT1632.drawImage(img, width,  height, (OUT_SIZE - width) / 2, 0);
  HT1632.render();
}
void clearLedCercle()
{
  for (int i = 0; i < 13; ++i)
  {
    setRegisterPin(i, LOW);
  }
  writeRegisters();
}

bool bBlink = false;
// Blink des LED de 0 à 12
void blinkled(){
  ledLoop++;
 for (int i = 0; i <= 13; i++)
 {
    if (bBlink)
      setRegisterPin(i, HIGH);
    else
      setRegisterPin(i, LOW);
 }
   bBlink = !bBlink;
    writeRegisters();  //MUST BE CALLED TO DISPLAY CHANGES
    startTimeLed = millis();
    if (ledLoop == 5)
      myLedState = noLED;
}
bool lstate = false;
// Chenille des LED de 0 à 12
void chenille()
{
  if (tLed > 12)
  {
    tLed = 0;
    ledLoop++;
  }
 if (ledLoop == 5)
 {
  myLedState = noLED;
 for (int i = 0; i <= 13; i++)
 {
      setRegisterPin(i, LOW);
 }
    writeRegisters();
 }
 else
 {
  for (int i = 0; i <= 13; i++)
 {
      setRegisterPin(i, LOW);
 }
  setRegisterPin(tLed, HIGH);
  tLed++;
  writeRegisters();
 }
  startTimeLed = millis();
}

void loop () {
  
  /*
     Returns the number of milliseconds since the Arduino board began running the current program. This number will overflow (go back to zero), after approximately 50 days.
  */
  unsigned long loopTimeAfficheur = millis() - startTimeAfficheur;
  unsigned long loopTimeLed = millis() - startTimeLed;
  unsigned long loopTimeRandom = millis() - startTimeRandom;
  unsigned long loopTimeLastCapsule = millis() - startTimeLastCapsule;
  bCapsule = false;
  
  /*
   * On regarde l'etat des capteurs IR
   * TODO Regarder les interrupt pour voir s'il est possible d'y foutre un signal.
   */
  a1ctivated = testPin(A1, a1ctivated);
  a2ctivated = testPin(A2, a2ctivated);

  if (bCapsule > 0 && !bCapsuleLoop && loopTimeLastCapsule >= DELAY_BETWEEN_CAPSULES)
  {
    compteurInc();
    bCapsule = 0;
    sprintf(text, "%d", cpt);
    //memcpy(text, cptC, sizeof(text));
    setScrolling();
    startTimeAfficheur = millis();
    startTimeRandom = millis();
    startTimeLastCapsule = millis();
    myTextState = AFFICHECOMPTEUR;
    myLedState = LEDCHENILLE;
    tLed = 0;
    ledLoop = 0;
  } 
   else
      bCapsuleLoop = false;
  /*
      Si l'etat en cours est l'affichage du compteur
  */
  if (myTextState == AFFICHECOMPTEUR)
  {
    // Si le texte a été affiché pendant DELAY_CENTERED_TEXT millisecondes.
    setScrolling();
    drawAdaptiveText();
      
    if (loopTimeAfficheur >= DELAY_CAPSULE_TEXT)
    {
      startTimeRandom = millis();
      startTimeAfficheur = millis();
      if (cpt%13 == 0) {
	      myTextState = AFFICHEPHRASECOMPTEUR;
	      getCapsuleText(text);
	      setScrolling();
      }
    }
    
  }
  /*
     Si l'etat en cours est l'affichage d'une phrase en réponse au compteur
  */
  else if (myTextState == AFFICHEPHRASECOMPTEUR || myTextState == AFFICHEPHRASE)
  {
    // Si le texte défile
    if (scrolling)
    {
      // Si il reste toujours du texte à afficher
      if (it <= wd + OUT_SIZE)
      {
        drawAdaptiveText();
        ledLoop = 0; // On clignotte tant que la phrase est affichée
        if (loopTimeAfficheur >= DELAY_SCROLLING_TEXT)
        {
          it++;
          startTimeAfficheur = millis();
          startTimeRandom = millis();
        }
      }
      // Si tout le texte a été affiché
      else
      {
        startTimeAfficheur = millis();
        startTimeRandom = millis();
        myTextState = AFFICHEIDLE;
      }
    }
    // Si le texte est statique
    else
    {
      drawAdaptiveText();
      // Si le texte a été affiché pendant DELAY_CENTERED_TEXT millisecondes.
      if (loopTimeAfficheur >= DELAY_CENTERED_TEXT)
      {
        startTimeAfficheur = millis();
        startTimeRandom = millis();
        myTextState = AFFICHEIDLE;
      }
    }
  }
  else if (myTextState == AFFICHEIDLE)
  {
    // Si ça fait plus de DELAY_RANDOM_TEXT qu'un texte n'a pas été affiché
   if (loopTimeRandom >= DELAY_RANDOM_TEXT)
   {
      startTimeRandom = millis();
      startTimeAfficheur = millis();
      myTextState = AFFICHEPHRASECOMPTEUR;
      getRandomText(text);
      setScrolling();
      myLedState = LEDBLINK;
    }
    else
    {
      sprintf(cptC, "%d", cpt);
      strcpy(text, cptC);
      setScrolling();
      drawAdaptiveText();
      myLedState = LEDCHENILLE;
    }
  }
  if (myLedState == noLED)
  {
    clearRegisters();
  }
  if (myLedState == LEDCHENILLE)
  {
    if (loopTimeLed >= DELAY_LED_CHENILLE)
    {
      chenille();
    }

  }
  if (myLedState == LEDBLINK)
  {
    if (loopTimeLed >= DELAY_LED_CHENILLE)
    {
     blinkled();
    }  
  }
}
