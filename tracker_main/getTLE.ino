void getTLE(int SAT){
  // Make HTTP request and get TLE for satellite//
  if (client.connect(server, 80)) {
    #ifdef DEBUG
      Serial.println("connected to server");
      Serial.println("Request #: " + String(SAT) + " For: " + String(satnames[SAT]));
    #endif
    // Make a HTTP request: //
    client.println("GET " + String(satURL[SAT]));
    client.println("Accept: */*");
    client.println("Host: www.celestrak.com");
    client.println("Connection: close");
  }
  int connectLoop = 0;
  while(client.connected()){
    int j = 0;
    while (client.available()) {
      char c = client.read(); 
      TLE[j] = c;   //store characters to string
      j++;
      connectLoop = 0;
    }
    k = 0;
    for (j=26; j<96; j++){  //TLE line 1 spans characters 26 - 96
      TLE1[SAT][k] = TLE[j];
      k++;
    }
    k = 0;
    for (j=97; j< 167; j++){  //TLE line 2 spans characters 97 - 167
      TLE2[SAT][k]= TLE[j];
      k++;
    }
    connectLoop++;
    delay(100);
    if(connectLoop > 100){
      client.stop();
      break;
    }
  }
}
