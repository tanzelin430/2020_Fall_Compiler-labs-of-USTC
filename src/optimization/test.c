int main() {
    int i,j;
    int u,v;
    i=1;
    u=3;
    v=20;
    while(v<=20){
        if( u < v){
            i=2;
            u=u+1;
        }
        v = v-1;
    }
    j=i;
    return 0;
}