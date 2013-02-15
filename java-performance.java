public class testprime {
  public static void main(String [] args) {
    System.out.println(2);
    for (int i=3;i<=10000;i++) {
      boolean prime = true;
      for (int j=2;j<i;j++) {
        int k = i/j;
        int l = k*j;
        if (l==i) prime = false;
      }
      if (prime) System.out.println(i);
    }
  }
}