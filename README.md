# Kirkmobilen

Mål: At lave en Kirk F68 om til mobil-telefon

# Forbindelser inde i telefonen

## Drejeskive

* Blå: common
* Rød: normal closed - open n x nummer man ringer
* Hvid(gul?): normal open, closed når drejeskiven er i bevægelse

## Telefonstik

* Venstre: Blå (3-L2)
* Top/Center: Gul (1-J)
* Højre: Rød (2-L1)

# Telefonrør

* Blå: Common (7-MT)
* Rød: Højtaler (8-T)
* Gul: Mikrofon (6-M)

# Klokke

* Sort: 12
* Gul: 9

# Kontakt v. rørløft

* 9 - 10: normal closed (lidt slidt?)
* 10 - 3-L2: normal closed
* 14 - 2-L1: normal closed

Når røret løftes, går alle i open.

# Links

* <http://www.instructables.com/id/Interface-a-rotary-phone-dial-to-an-Arduino/>
* <https://nesdunk.dk/?cat=14>
* <https://www.sparkfun.com/products/retired/286> (bla. schematic til at få ringeren til at virke ser det ud til!)
* <https://www.sparkfun.com/tutorials/51>
* <https://www.stavros.io/posts/irotary-saga/>
* <https://hackaday.com/2019/02/05/this-vintage-phone-goes-cellular/>
* <https://www.instructables.com/id/TOWA-Phone/>
* <https://github.com/Trasselfrisyr/SLIC-GSM-rotary-phone/blob/master/SLIC-GSM-rotary-phone.ino> Kode!

# Klokke

* <https://www.sparkfun.com/tutorials/50> (Få klokken til at virke)

Fra kommentarene:

## SN754410 H bridge explained
You supply two low voltage block signals; one to 1A and one to 2A. The block signals are 180 degrees out of phase. The block signals are relative to GND. By supplying a large voltage on VCC2, outputs 1Y and 2Y will provide a high voltage symmetrical block wave like so:
```
1A: _|¯¯¯|___|¯¯¯|___|¯¯¯|_ +5V DC relative to GND

2A: ¯|___|¯¯¯|___|¯¯¯|___|¯ +5V DC relative to GND (180 degrees out of phase compared to the signal on 1A)

1Y: _|¯¯¯|___|¯¯¯|___|¯¯¯|_ +35V DC relative to GND

2Y: ¯|___|¯¯¯|___|¯¯¯|___|¯ +35 DC relative to GND (180 degrees out of phase compared to the signal on 1Y)
```

By connecting the ringer directly to 1Y and 2Y you now effectively have a -35V DC to +35V DC signal.

STEP 1 - You can program an Arduino to generate two 5V 22Hz block waves 180 degrees out of phase like so:

```
void setup() {
   //Phone ring signal is 22Hz typically.  20Hz is used in this example.
   //20Hz=(1/20)*1000 = 50 milliseconds period
  //50 milliseconds period = 25ms HIGH + 25ms LOW
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
}

void loop() {
  //Ring pattern: RING,RING.....RING,RING.....
  //RING for one second (20x 50ms = 1 sec)

  //RING 1
  for (int i=0; i <= 20; i++) {
    digitalWrite(12, HIGH);
    digitalWrite(13, LOW);
    delay(25);
    digitalWrite(12, LOW);
    digitalWrite(13, HIGH);
    delay(25);
  }
  //PAUSE for 0.5 seconds between first and second ring.
  digitalWrite(12, LOW);
  digitalWrite(13, LOW);
  delay(500);
  //RING 2
  for (int i=0; i <= 20; i++) {
    digitalWrite(12, HIGH);
    digitalWrite(13, LOW);
    delay(25);
    digitalWrite(12, LOW);
    digitalWrite(13, HIGH);
    delay(25);
  }
  //PAUSE for 1.5 seconds
  digitalWrite(12, LOW);
  digitalWrite(13, LOW);
  delay(1500);
}
```

You should now see the LED on pin13 flash at 20Hz for 1 second twice in rapid succession followed by a 1.5 second pause. pin12 carries the same signal, but 180 degrees out of phase.

STEP 2 - Build the SN754410 ‘push-pull’ symmetrical block wave circuit
Simply use VCC = VCC1 = VCC2 = 5V from your Arduino for now.

STEP 3 - DC/DC converter
Buy a 12V in 30V-60V-90V out DC-DC converter from eBay to provide a high voltage to VCC2 instead of the +5V DC to boost the output signal on 1Y/2Y.

STEP 4 - Increase VCC2
Finally replace VCC2 with a higher DC voltage not exceeding 35V. 1Y/2Y signal amplitude should increase accordingly.

Note: SN754410 is rated to 35V DC as stated in previous comments. Order more than one SN754410 if you are going to attempt to build this circuit.

I was able to make it work just using the SN754410 + eBay DC converter (measured output of 50V DC) + Arduino without any passive components (i.e. capacitors) at all. Just keep your finger on the SN754410 to feel if it is getting hot or not (and connect a heatsink if need be).