import java.util.*; 


public class LinearSelect {
    public static int LinearSelectCount =0; 
    public static int MofMcount =0; 

    
    // returns the kth smallest element in S
    static int LinearSelect(int[] S, int k) {
        System.out.println("this is the " + LinearSelectCount+ " th iteration of Linear Select and the list size is "+ S.length); 

        LinearSelectCount ++; 
       // 
       /*
        * if 𝒏 = 𝟏 then
return 𝑺
𝒑 ← 𝑀𝑒𝑑𝑖𝑎𝑛𝑂𝑓𝑀𝑒𝑑𝑖𝑎𝑛𝑠 𝑺
𝑳, 𝑬, 𝑮 ← 𝑝𝑎𝑟𝑡𝑖𝑡𝑖𝑜𝑛(𝑺, 𝒑)
if 𝒌 ≤ 𝑳 then
return 𝐿𝑖𝑛𝑒𝑎𝑟𝑆𝑒𝑙𝑒𝑐𝑡(𝑳, 𝒌)
else if 𝒌 ≤ 𝑳 + 𝑬 then
return 𝒑
else
return 𝐿𝑖𝑛𝑒𝑎𝑟𝑆𝑒𝑙𝑒𝑐𝑡 𝑮, 𝒌 − 𝑳 − 𝑬
        */
        if(S.length ==1){
            System.out.println("\n\nLS finished and found the "+k+"th element and it is "+S[0]+"\n");
            return S[0]; 
        }
        if(S.length==0){
            System.out.println("Something went very wrong in Linear Select"); 
            return 9999; 
        }
    
        int pivot = (int)MedianOfMedians(S);
        ArrayList<Integer> L= new ArrayList<>(); 
        ArrayList<Integer> E = new ArrayList<>(); E.add(pivot); 
        ArrayList<Integer> G = new ArrayList<>(); 


        
        for(int i =0; i < S.length; i ++){
            if(S[i]< pivot){
                L.add(S[i]); 
            }else if (S[i]> pivot){
                G.add(S[i]); 
            }else{
                E.add(S[i]);
            }
        }

        if(k<= L.size()){
            int[] LArray = new int[L.size()];
            System.out.println("calling on L with size " + L.size()+" and the original Array was " +Arrays.toString(S)+ " and the pivot was "+ pivot);
            for (int i = 0; i < LArray.length; i++){
                LArray[i] = L.get(i);  
            }     
            return LinearSelect(LArray, k);
        } else  if(k<= L.size()+ E.size()){
            return pivot; 
        }else {
            int[] GArray = new int[G.size()];
            for (int i = 0; i < GArray.length; i++){
                GArray[i] = G.get(i);  
            } 
            System.out.println("calling on G with size "+ G.size()+" and the original Array was " +Arrays.toString(S)+ " and the pivot was "+ pivot);
    
            return LinearSelect(GArray, k-L.size()-E.size());
        }
    }






    //Finds the median value in a list of 5 (or less)
    static int findMedian(int[] arrayOf5){
        System.out.println("Finding the median of "+ Arrays.toString(arrayOf5)); 

        int[] sorted = new int[arrayOf5.length];
        for (int i =0; i < arrayOf5.length; i ++){
            int min = arrayOf5[i];
            int minIDX = i; 
            System.out.println("Current min is "+ min); 
            for(int j =i+1; j < arrayOf5.length; j ++){
                if( arrayOf5[j]< min){
                    min = arrayOf5[j];
                    minIDX = j; 
                    System.out.println("found new min "+ min);
                }
            }
            sorted[i]= min;
            arrayOf5[minIDX]= arrayOf5[i];

        }
        int median = sorted[(int)Math.ceil((arrayOf5.length/2))];
        System.out.print("\nthe sorted list is: "+Arrays.toString(sorted)); 
        
        // for(int a =0; a< sorted.length; a++){
        //     System.out.print(sorted[a]);
        // }
        System.out.println("\nthe median is " +median); 
        return median; //return median of 5
    }





    // returns the median of medians pivot (with groups of size 5) of S
    static int MedianOfMedians(int[] S) {
        if(S.length==0){
            System.out.println("Something went very wrong in Medians of medians"); 
            return 99; 
        }
        MofMcount++; 
        System.out.println("This is Medians of medians iteration "+ MofMcount+ " and the array length is "+ S.length); 
        int pivot = 999;
        if(S.length ==1){
            return S[0];
        }

        /*
        𝑴𝒆𝒅𝒊𝒂𝒏𝑶𝒇𝑴𝒆𝒅𝒊𝒂𝒏𝒔(𝑺):
Divide 𝑺 into ڿ ۀ𝒏/𝟓 subsequences, 𝑺𝟏, ... , 𝑺ڿ ۀ𝒏/𝟓 , of size 𝟓
// or ڿ ۀ𝒏/𝟕 subsequences of size 𝟕
for 𝒊 ← 𝟏 to ڿ ۀ𝒏/𝟓 do
𝒙𝒊 ← median of 𝑺𝒊
𝒑 ← 𝐿𝑖𝑛𝑒𝑎𝑟𝑆𝑒𝑙𝑒𝑐𝑡 𝒙𝟏, ... , 𝒙𝒈 , ቒ ቓ
ڿ ۀ𝒏/𝟓
𝟐
return 𝒑
         */
        int g =0;
        // System.out.println("S length is " + S.length+ " in Medians of medians"); 
        double DnumMedians = (S.length/5.0);
        System.out.println("double num medians is "+DnumMedians);

        int numMedians = (int)Math.ceil(DnumMedians);
        System.out.println("int num medians is "+numMedians);
        int[] ArrayOfMedians = new int[numMedians];
        int[] group = new int[5];
        for(int i =0; i < S.length; i ++){//make little arrays and sort it
            if(g == numMedians -1){//if this is the last median we will find
                if(i>0){
                    i--;
                }
                System.out.println("this is the last group and might not have 5 elm\ni is "+ i+" and S[i] is "+S[i]); 
                // System.out.println()
                
                int[] last = new int[S.length - i]; //this is an important place to
                // check edge cases to make sure the indexes are correct
                
                for(int j =0; j < last.length; j ++){
                    last[j]= S[i];
                    i ++; 
                }
                ArrayOfMedians[g]= findMedian(last);
                g ++;
                break; 
            }else {//not the last groups

                if (i % 5 == 0 && i != 0) {
                    ArrayOfMedians[g] = findMedian(group);
                    g++;
                }
                group[i % 5] = S[i];
               // System.out.println("i is "+ i+" and S[i] is "+S[i]); 

            }
        }//end of for loop
        //call LS here
        System.out.println("Calling Linear Select on: "+Arrays.toString(ArrayOfMedians)); 
        // for(int a =0; a< ArrayOfMedians.length; a++){
        //     System.out.print(ArrayOfMedians[a]);
        // }
        // System.out.println("}");
        pivot = (int)LinearSelect(ArrayOfMedians, (int)Math.ceil(numMedians/2));
        System.out.println("pivot is "+pivot);  

        return pivot;
    }
    
    public static void main(String[] args) {
        System.out.println("starting main");
        // Example input
        int[] S = {8, 3, 13, 16, 20, 9, 1, 3, 11, 17, 10, 18, 12, 4, 15, 8, 15, 6, 21, 7, 5, 14, 20, 2, 19, 22, 3};
        // Sorted: 1, 2, 3, 3, 3, 4, 5, 6, 7, 8, 8, 9, 10, 11, 12, 13, 14, 15, 15, 16, 17, 18, 19, 20, 20, 21, 22
          // int [] S= {1, 2, 3, 4, 5, 6, 7, 8};//two groups, size it 9
        //int[] S = {1, 2, 3,4, 5}; 
        
         System.out.println("the 19th smallest element is" +LinearSelect(S, 19)); // the 19th smallest number in S is 15
        
        //System.out.println("the 5th smallest element is" +LinearSelect(S, 5)); // the 1st smallest number in S is 1

     //   System.out.println(LinearSelect(S, 27)); // the 27th smallest number in S is 22
        
    }
}
    