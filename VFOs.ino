//------------
void set_vfoA()             // 'A' key
{
  long kpdEntry = millis();    //time of entering this fn
  while(Kpd.Key_State() == 3)    // key remains pressed > 2 sec: save freq
  { 
    while(Kpd.Key_State() == 3)
    { }
    if(millis() > kpdEntry + 2000);
    store_freq();
    return;
  }
  // bindex = 4;    // 40m
  VFO_mode = true;
  actvVFO = 'A';
  //currFreq = BandWkgFreqs[bindex];
  load_freq();
}

void set_vfoB()             // 'B' key
{
  long kpdEntry = millis();    //time of entering this fn
  while(Kpd.Key_State() == 3)    // key remains pressed > 2 sec
  { 
      while(Kpd.Key_State() == 3) {}
    if(millis() > kpdEntry + 2000);
    store_freq();
    return;
  }
  //bindex = 6;    // 20m
  VFO_mode = true;
  actvVFO = 'B';
  //currFreq = BandWkgFreqs[bindex];
  load_freq();
}

void set_vfoC()             // 'C' key
{
  long kpdEntry = millis();    //time of entering this fn
  while(Kpd.Key_State() == 3)    // key remains pressed > 2 sec
  { 
    while(Kpd.Key_State() == 3) {}while(Kpd.Key_State() == 3)
    if(millis() > kpdEntry + 2000);
    store_freq();
    return;
  }
  //bindex = 8;    // 15m
  VFO_mode = true;
  actvVFO = 'C';
  //currFreq = BandWkgFreqs[bindex];
 load_freq();
}


void set_vfoD()             // 'D' key
{
  long kpdEntry = millis();    //time of entering this fn
  while(Kpd.Key_State() == 3)    // key remains pressed > 2 sec
  { 
      while(Kpd.Key_State() == 3) {}
    if(millis() > kpdEntry + 2000);
    store_freq();
    return;
  }
  //bindex = 10;    // 10m
  VFO_mode = true;
  actvVFO = 'D';
  //currFreq = BandWkgFreqs[bindex];
 // load_freq();
}
