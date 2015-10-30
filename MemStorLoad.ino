
void store_freq()
{
  int epromAddress = 0; //The base address
  currInfo.currFreq_s = currFreq;
  currInfo.if_offset_s = if_offset;
  currInfo.ritFreq_s = ritFreq; 
  currInfo.currSB_s= currSB; 
  currInfo.ssb_offset_s = ssb_offset;

  if(!VFO_mode)
    epromAddress = epromAddress + currCh * sizeof(currInfo); // in mem mode ch no x  n bytes info for each ch
  else
  { 
    switch(actvVFO)
    {
    case 'A':
      epromAddress = VFObaseAdd; // base address for A VFO
      break;

    case 'B':
      epromAddress = VFObaseAdd + sizeof(currInfo);
      break;

    case 'C':
      epromAddress = VFObaseAdd + 2 * sizeof(currInfo);
      break;

    case 'D':
      epromAddress = VFObaseAdd + 3 * sizeof(currInfo);
      break;
    }
  }

  EEPROM_writeAnything(epromAddress, currInfo);
  lcd.setCursor(msgCol, msgRow);
  lcd.print("Stored");
  delay(500); 
}

//----------------
void load_freq()
{
  int epromAddress = 0; //The base address
  if(!VFO_mode)
    epromAddress = epromAddress + currCh * sizeof(currInfo); // ch no x n bytes info for each ch
  else
  { 
    switch(actvVFO)
    {
    case 'A':
      epromAddress = VFObaseAdd; // base address for A VFO
      break;

    case 'B':
      epromAddress = VFObaseAdd + sizeof(currInfo);
      break;

    case 'C':
      epromAddress = VFObaseAdd + 2 * sizeof(currInfo);
      break;

    case 'D':
      epromAddress = VFObaseAdd + 3 * sizeof(currInfo);
      break;
    }
  }
  EEPROM_readAnything( epromAddress, currInfo);
  currFreq = currInfo.currFreq_s;
  if_offset = currInfo.if_offset_s;
  ritFreq = currInfo.ritFreq_s; 
  currSB= currInfo.currSB_s;       
  ssb_offset = currInfo.ssb_offset_s;
}

void storeLastSettings()
{
  int epromAddress = VFObaseAdd + 4 * sizeof(currInfo);
  EEPROM.write(epromAddress, VFO_mode);
  epromAddress++;
  EEPROM.write(epromAddress, actvVFO);
  epromAddress++;
  EEPROM.write(epromAddress, currCh);
  lcd.setCursor(msgCol, msgRow);
  lcd.print("Stored");
  delay(500); 
}

void loadLastSettings()
{
  int epromAddress = VFObaseAdd + 4 * sizeof(currInfo);
  VFO_mode = EEPROM.read(epromAddress);
  epromAddress++;
  actvVFO = EEPROM.read(epromAddress);
  epromAddress++;
  currCh = EEPROM.read(epromAddress);
}


