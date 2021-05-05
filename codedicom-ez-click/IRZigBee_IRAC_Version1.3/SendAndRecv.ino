//////////////////////////////////SAVING AND TAKING DATA FROM THE FLASH MEMORY(ESP8266)/////////////////////////

/*
    This creates two empty databases, populates values, and retrieves them back
    from the SPIFFS file system.
*/

void WiFiOff() {
   wifi_station_disconnect();
   wifi_set_opmode(NULL_MODE);
   wifi_set_sleep_type(MODEM_SLEEP_T);
   wifi_fpm_open();
   wifi_fpm_do_sleep(0xFFFFFFF);
}

const char* data = "Callback function called";
static int callback(void *data, int argc, char **argv, char **azColName) {
   int i;
   Serial.printf("%s: ", (const char*)data);
   for (i = 0; i<argc; i++){
       Serial.printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   Serial.printf("\n");
   return 0;
}

int db_open(const char *filename, sqlite3 **db) {
   int rc = sqlite3_open(filename, db);
   if (rc) {
       Serial.printf("Can't open database: %s\n", sqlite3_errmsg(*db));
       return rc;
   } else {
       Serial.printf("Opened database successfully\n");
   }
   return rc;
}

char *zErrMsg = 0;
int db_exec(sqlite3 *db, const char *sql) {
   Serial.println(sql);
   long start = millis();
   int rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);
   if (rc != SQLITE_OK) {
       Serial.printf("SQL error: %s\n", zErrMsg);
       sqlite3_free(zErrMsg);
   } else {
       Serial.printf("Operation done successfully\n");
   }
   Serial.print(F("Time taken:"));
   Serial.println(millis()-start);
   return rc;
}
void save_data_file(String id ,String at ) {
  sqlite3 *db1;
  int rc;
  String insert_info = "INSERT INTO save_data_1 VALUES ("; //EX :"INSERT INTO save_data_1 VALUES (1, 'AT;1;2;3;4;5;3');" 
  insert_info = insert_info + id + ", '" + at + "');";
  String select_from = "SELECT ";//WHERE "; // EX: "SELECT * FROM save_data_1 WHERE DICOMIOTID "
  select_from =select_from + id + " FROM save_data_1 ";
   rc = db_exec(db1, insert_info.c_str() );
   if (rc != SQLITE_OK) {
       sqlite3_close(db1);
       return;
   }
   rc = db_exec(db1, select_from.c_str() );      
   if (rc != SQLITE_OK) {
       sqlite3_close(db1);
       return;
   }
   sqlite3_close(db1);
   Serial.write(id.c_str());
}

//// CUT String + split case (case 1(air con lib have) inside Getinfo , case 2 (learning and saving) + case 3 (send only) upstair  )

stdAc::state_t now_state;
stdAc::state_t prev_state;
int Get_info(String s) {
 // Serial.println("get start");
 // Serial.println("");
  uint8_t a[7];
  String password;
  int j = 0 ;
  int b[9];
  char buff[20];
  bool power ;
  //String s = "AT;1;;;;;;;";//AT(start), Quest(1: send the known condition; 2: Study; 3: Resend status only), Protocol, Mode, Temp, Fan, Swing)
  if (s.substring(0,2) == "AT" ) {
    String cut = s.substring(2,s.length()+1);
    for (int i = 0 ; i <= cut.length() ; i++ ) {
      if(cut.substring(i,i+1) == ";") {
        b[j] = i;
        j++;
      }
    }
    a[0] = cut.substring(b[0]+1,b[1]).toInt();
    if(a[0] == 1 ) {
      
      for (int q = 1 ; q <8 ; q++ ) {
        a[q] = cut.substring(b[q]+1,b[q+1]).toInt();
      }
    }
    if(a[0] == 2 ) {
      id_saving = cut.substring(b[1]+1,b[2]);
    }
    if(a[0] == 4 ) {
      password = cut.substring(b[1]+1,b[2]);
    }
  }
  if (a[0] == 1 ) { // If it is known air conditioner
    if (a[2] == 0 ) {
      power = prev_state.power;
    }
    if (a[2] == 1 ) {
      power = true ;
    }
    else if ( a[2] == 2 ) {
      power = false;
    }
    if(a[3] == 0 ) {
      a[3] =(int)prev_state.mode+2; 
    }
    if(a[4] == 0 ) {
      a[4] =(int)prev_state.degrees;
    }
    if(a[5] == 0 ) {
      a[5] =(int)prev_state.fanspeed+2;
    }
    if(a[6] == 0 ) {
      a[6] =(int)prev_state.swingv+2;
    }
    irac.initState(&now_state,(decode_type_t)a[1],(int16_t)0,power,(stdAc::opmode_t)(a[3]-2),a[4],true,(stdAc::fanspeed_t)(a[5]-2),(stdAc::swingv_t)(a[6]-2),(stdAc::swingh_t)0,false,false,false,false,false,false,false,(int16_t)-1,(int16_t)-1);
    irac.sendAc(now_state,&now_state);
    //irac.sendAc((decode_type_t)a[1],(int16_t)0,power,(stdAc::opmode_t)a[3],a[4],true,(stdAc::fanspeed_t)a[5],(stdAc::swingv_t)a[6],(stdAc::swingh_t)0,false,false,false,false,false,false,false,(int16_t)-1,(int16_t)-1); 
    Serial.write(s.c_str());
    
      //Serial.println("------------");
      //Serial.println(sendACJsonState(&now_state));
      //Serial.println("  ");
    led_status();
      // luu tru prev_state
    //convert_state(now_state,prev_state); // can't change
    prev_state.protocol  = now_state.protocol ;
    prev_state.model     = now_state.model ;
    prev_state.power     = now_state.power ;
    prev_state.mode      = now_state.mode ;
    prev_state.degrees   = now_state.degrees ;
    prev_state.celsius   = now_state.celsius ;
    prev_state.fanspeed  = now_state.fanspeed ;
    prev_state.swingv    = now_state.swingv ;
    prev_state.swingh    = now_state.swingh ; 
    prev_state.quiet     = now_state.quiet ;
    prev_state.turbo     = now_state.turbo ;
    prev_state.econo     = now_state.econo ; 
    prev_state.light     = now_state.light ; 
    prev_state.filter    = now_state.filter ;
    prev_state.clean     = now_state.clean ;
    prev_state.beep      = now_state.beep ; 
    prev_state.sleep     = now_state.sleep;
  }
  if (a[0] == 2 ) {//learning and saving
    //unsigned long mills_start = millis();
    //if( (millis()-mills_start) >=10000 ) {
    //  return;
    
    //}
  }
  if (a[0] == 3 ) { // resend newsletter
    //file_load();
  }
  if (a[0] == 4) {
    if (password =="ABC") {
      //Serial.println("Remove file save ");
      SPIFFS.remove("/FLASH/save_data_1.db"); //delete failed 
      SPIFFS.remove("save_data_1.db");
    }
    else {
      //Serial.println("Wrong pass ");
    }
  }
  return a[0];
} 
//int buffering[360];
/*void save_mission() {  
  if (id_saving != "") {
    //Serial.println(id_saving);
    if ( irrecv.decode(&action_recv())) {  
     // String description = IRAcUtils::resultAcToString(&results);
      yield();  // Feed the WDT (again)
      //Serial.write(description.c_str());
    Serial.println();    // Blank line between entries
    yield();             // Feed the WDT (again)
      //Serial.println("  ");
      //Serial.print(system_get_free_heap_size());// con` free bao nhieu bo nho heap
      //Serial.println("  ");
      yield();  // Feed the WDT (again)
      save_data_file(id_saving,only_raw_code(&results));
    }
  }
}*/
