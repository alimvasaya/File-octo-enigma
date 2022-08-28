#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <semaphore.h>
#include <unistd.h>  
#include <pthread.h>
#include <iostream>
#include <cmath>

using namespace std;

// check values function checks for size of binary string
// and add 0 if the size is less than max bit size.
//  bitsize - string size = amount of 0's

struct Main_Values
{  
    int *dec;
    int index;
    char *alpha;
    string *msg;

    int *bits;
    int *turn;
    
    pthread_cond_t *c_empty; 
    pthread_mutex_t *c_bsem;
    sem_t *c_nsem;
    
    string *binary_values;
    char *characters;
};

struct Mthread{

    string *dec;
    char *alpha;
    string *msg;

    int index;

    int *bits;
    int *turn;

    pthread_cond_t *c_empty; 
    pthread_mutex_t *c_bsem;
    sem_t *c_nsem;

    string output;
};

string check_Values(int bit_size, string binary_value)
{
    int size = binary_value.length();
    string temp;

    if(bit_size > size){
        for(int i = 0; i < bit_size-size; i++)
        {
            temp = binary_value;
            binary_value = '0';
            binary_value += temp;
        }
    }    
    return binary_value;
}
// max bit functions, to look for max bits.

int Find_Bits(int maxVal) 
{
  return ceil(log2(maxVal));
}
// dec to binary function, takes in one decimal at a time.
// return the binary value for a given decimal integer.
// returns a string of binary in reverse order.

string DecimalToBinary(int num)
{
	string str;
	while(num){
	if(num & 1) // 1
		str+='1';
	else // 0
		str+='0';
	num>>=1; // Right Shift by 1
	}
	return str;
}

// reverse function takes in the string value and resverse the string
// using this for dec to binary function to return decompressed values
// in the right order.

string reverse(string str)
{
    string binary_values , output;

    for(int i=str.size()-1 ; i>=0 ; i--)
    {
         binary_values = str[i];
         output += binary_values;
    }
    return output;
}

// max value function, helps look for max value from the given array.

int Max_value(int DecArr[], int n)
{
  int Maxvalue = DecArr[0];

  for (int i = 0; i < n; ++i) 
  {
    if (DecArr[i] > Maxvalue) 
    {
        Maxvalue = DecArr[i];
    }
  }
  return Maxvalue;
}


// first thread starts here

void* Thread1(void *arg){

  struct Main_Values *values = (struct Main_Values*) arg;

    // Semaphore
    

    int *c_turn = values->turn;
    int *bit_size_val = values->bits;
    char *alphabet = values->alpha;
    int *decimal = values->dec;
    string *msg = values->msg;
    string *binary_values_ptr = values->binary_values;
    
    
    sem_post(values->c_nsem);
    
     // Critical Section
    pthread_mutex_lock(values->c_bsem);
 
    // Wait for another thread 
    while (values->index != *values->turn)
    {
        pthread_cond_wait(values->c_empty, values->c_bsem);
            // convert dec to binary and store it
    }
    pthread_mutex_unlock(values->c_bsem);

    string binary_converted_values = reverse(DecimalToBinary(*decimal));

    string binary_checked_values = check_Values(*bit_size_val,binary_converted_values);  
    
    int frequency = 0;

    
    for(int i = 0; i<= msg->size()-1; i=i+*bit_size_val)
    {
       string Binary_value = msg->substr(i,*bit_size_val);
    // cout << "true" << endl;

       if(Binary_value == binary_checked_values)
       {
           frequency++;
       }
      
    }
     
    if(*values->turn==0)
    {
        cout << "Alphabet:\n";
    }
    values->binary_values = &binary_checked_values;
    values->characters = alphabet;

    cout << "Character: " <<*alphabet << ", code: " <<binary_checked_values<< ", Frequency: " <<frequency << endl;
    *binary_values_ptr = binary_checked_values;
    // Critical Section
    pthread_mutex_lock(values->c_bsem);
    *values->turn = *values->turn + 1;
    
    // Wake up thread which is waiting in main
    pthread_cond_broadcast(values->c_empty);
        
    // Unlock and end critcal section
    pthread_mutex_unlock(values->c_bsem);
    
    return NULL;
}

 void* Thread2(void* arg) 
 {
    struct Mthread *values = (struct Mthread*) arg;

   
    int *c_turn = values->turn;
    int *bit_size_val = values->bits;
    char *alphabet = values->alpha;
    string *binary_values_from_main = values->dec;

    string *msg = values->msg;
    sem_post(values->c_nsem);
    
    
     // Critical Section
    pthread_mutex_lock(values->c_bsem);
 
    // Wait for another thread 
    while (values->index != *values->turn)
    {
        pthread_cond_wait(values->c_empty, values->c_bsem);
            // convert dec to binary and store it
    }
    pthread_mutex_unlock(values->c_bsem);


    string  *Decompressed_Message;

    
    for(int i = 0; i< msg->size(); i=i+*bit_size_val)
    {
       string Binary_value = msg->substr(i,*bit_size_val);


       if(Binary_value == *binary_values_from_main)
       {
           *Decompressed_Message = alphabet;
       }
    }
    values->output = *Decompressed_Message;

    // Critical Section
    pthread_mutex_lock(values->c_bsem);
    *values->turn = *values->turn + 1;
    
    // Wake up thread which is waiting in main
    pthread_cond_broadcast(values->c_empty);
        
    // Unlock and end critcal section
    pthread_mutex_unlock(values->c_bsem);
    
    return NULL;
}

//  -------------Main-----------------


int main(int argc, char *argv[])
{
    //Declare semaphores variables
    static pthread_mutex_t bsem; 
    static pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
    sem_t nsem;

   

    pthread_mutex_init(&bsem, NULL);
    struct Main_Values arg;
    int Number_of_alphabets, max_BIT, val; 
    string Encoded_Message, s;

    cin >> Number_of_alphabets;
    cin.ignore();
    
     // arrays to store values from the file

     char *List_of_Alphabets = new char[Number_of_alphabets] ;
     int *List_of_Decimal_Notations = new int[Number_of_alphabets];


     string *binary_values_from_thread;
     char *chracters_from_thread;
     int index_from_thread;

  
// storing file input values to variables

     
    for(int i =0; i<Number_of_alphabets; i++)
    {
        getline(cin, s);

        List_of_Alphabets[i] = s[0];
      
        List_of_Decimal_Notations[i] = stoi(s.substr(2));
    }   

    if (List_of_Alphabets == nullptr &&  List_of_Decimal_Notations == nullptr)
    {
         cout << "Error: memory could not be allocated";
         return -1;
    }
      
    val = Find_Bits(Max_value(List_of_Decimal_Notations,Number_of_alphabets));

    cin >> Encoded_Message;
    cin.ignore();
    int message_size = Encoded_Message.length()/val;


    //thread starts 
    int turn = 0;
    pthread_t *tid = new pthread_t[Number_of_alphabets];

     // Semaphore (mutex)
    arg.c_empty = &empty;
    arg.c_bsem = &bsem;
    arg.turn = &turn;
    arg.msg = &Encoded_Message;
    arg.bits = &val;
    
    // Semaphore
    arg.c_nsem = &nsem;
    sem_init(arg.c_nsem, 0, 0);
    string binary_values[Number_of_alphabets];
    
    for (int i = 0; i <= Number_of_alphabets-1; i++)
    {
        arg.alpha = &List_of_Alphabets[i];
        arg.dec = &List_of_Decimal_Notations[i];
        arg.index = i;
        arg.binary_values = &binary_values[i];
        
        if (pthread_create(&tid[i], nullptr, Thread1, &arg))
        {
            cerr << "Cannot create the thread" << endl;
            return 1;
        }
        
        // Wait for another thread 
        sem_wait(arg.c_nsem);
    }
    index_from_thread = arg.index;
    binary_values_from_thread = new string [index_from_thread];
    chracters_from_thread = new char[index_from_thread];


    for (int i = 0; i < Number_of_alphabets-1; i++)
    {
        pthread_join(tid[i], nullptr);
        binary_values_from_thread[i]= *arg.binary_values;
        chracters_from_thread[i] = *arg.characters;   
    }
    
    delete[] tid; 
    cout << " " << endl;

//thread 2 starts 
    
     turn = 0;
     pthread_t *tid2 = new pthread_t[index_from_thread];
    struct Mthread arg2;
     // Semaphore (mutex)
    arg2.c_empty = &empty;
    arg2.c_bsem = &bsem;
    arg2.turn = &turn;
    arg2.msg = &Encoded_Message;
    arg2.bits = &val;
    string output;
    // Semaphore
    arg2.c_nsem = &nsem;
    sem_init(arg2.c_nsem, 0, 0);
    
    for (int i = 0; i < index_from_thread; i++)
    {
        arg2.alpha = &chracters_from_thread[i];
        arg2.dec = &binary_values_from_thread[i];
        arg2.index = i;
        
        if (pthread_create(&tid2[i], nullptr, Thread2, &arg2))
        {
            cerr << "Cannot create the thread" << endl;
            return 1;
        }
        // Wait for another thread 
        sem_wait(arg2.c_nsem);
    }
    for (int i = 0; i < index_from_thread; i++)
    {
        pthread_join(tid2[i], nullptr);
       output  = arg2.output; 
    }
    cout << "Decompressedn Message" << output << endl;
    delete[] tid2; 

return 0;

}
