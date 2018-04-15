#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
//#include <gtk/gtk.h>
#include <unistd.h> //nanosleep(&ps, NULL); struct timespec ps;

int ArBase[4194304]={0};
int ArPant[16793604]={0};
int BaseF=1;
int BaseC=1;
int Entry=0;
int Exit=0;
int ready=0;

int SpwTrR[2796203]={0};
int SpwTrC[2796203]={0};
int stp;

char* archivo;

GtkDialog* mnsjResolv;
GtkDialog* Confirm;
GtkFileChooserDialog* DA;
GtkFileChooserDialog* DG;
GtkWidget* window;
GtkFileChooser* ABRIR;
GtkFileChooser* GUARDAR;
GtkSpinButton* filas;
GtkSpinButton* columnas;

struct entrMod{GtkEntry* entry; int dato;};

int corriendo=0;
int solucion;
int lst;

int eab=0;

char *TRes;

int guardar();
int abrir();
void Pasar();

void set_basef();
void set_basec();

int resolv(int m,int n,int ar[][n]);
void addFrontier(int m, int n,int i, int j,int ar[][n]);
void removeBarrier(int m, int n,int i, int j,int ar[][n]);
void setentrys(int m, int n,int ar[][n]); //n m > 1
void generate_maze();
void desplegar ();
int abrir_maze(char* archivo);
void guardar_maze(char* archivo);

int main(int argc, char *argv[])
{
    GtkBuilder      *builder;

    gtk_init(&argc, &argv);

    builder = gtk_builder_new();
    gtk_builder_add_from_file (builder, "inter.glade", NULL);

    window = GTK_WIDGET(gtk_builder_get_object(builder, "window_main"));

    srand(time(NULL));

	TRes= g_new (gchar,20);

	mnsjResolv=GTK_DIALOG(gtk_builder_get_object(builder, "msj_Resolver"));
	Confirm=GTK_DIALOG(gtk_builder_get_object(builder, "Confirm"));
	filas=GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "spinbuttonFilas"));
    columnas=GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "spinbuttonColumnas"));
    DA=GTK_FILE_CHOOSER_DIALOG(gtk_builder_get_object(builder,"fileAbrir"));
    DG=GTK_FILE_CHOOSER_DIALOG(gtk_builder_get_object(builder,"fileGuardar"));
    ABRIR=GTK_FILE_CHOOSER(DA);
	GUARDAR=GTK_FILE_CHOOSER(DG);;

    Solucionable=GTK_LABEL(gtk_builder_get_object(builder, "Solucionable"));

    gtk_builder_connect_signals(builder, NULL);

    g_object_unref(builder);

    gtk_widget_show(window);
    gtk_main();

    return 0;
}

int resolv(int m,int n,int ar[][n]){
    int spawn = rand() % (n*m);
    int mov;
    int i,j;
    stp=0;
    Entry=Exit=0;
    for(i=0;i<m;i++){
        for(j=0;j<n;j++)
            ar[i][j]=0;
    }
    i=spawn/n;
    j=spawn%n;
    ar[i][j]=16;
    addFrontier(m,n,i,j,ar);
    while(stp>0){
        mov=0;
        if(stp>0)
            mov = rand()%stp;
        i=SpwTrR[mov];
        j=SpwTrC[mov];
        SpwTrR[mov]=SpwTrR[stp-1];
        SpwTrC[mov]=SpwTrC[stp-1];
        stp--;
        removeBarrier(m,n,i,j,ar);
        addFrontier(m,n,i,j,ar);
    }
    i=spawn/n;
    j=spawn%n;
    ar[i][j]-=16;
    setentrys(m,n,ar);
    return 1;
}

void addFrontier(int m, int n,int i, int j,int ar[][n]){
    if (0<=i-1){
        if(ar[i-1][j]==0){
            SpwTrR[stp]=i-1;
            SpwTrC[stp]=j;
            ar[i-1][j]=-1;
            stp++;
        }
    }
    if (0<=j-1){
        if(ar[i][j-1]==0){
            SpwTrR[stp]=i;
            SpwTrC[stp]=j-1;
            ar[i][j-1]=-1;
            stp++;
        }
    }
    if (i+1<m){
        if(ar[i+1][j]==0){
            SpwTrR[stp]=i+1;
            SpwTrC[stp]=j;
            ar[i+1][j]=-1;
            stp++;
        }
    }
    if (j+1<n){
        if(ar[i][j+1]==0){
            SpwTrR[stp]=i;
            SpwTrC[stp]=j+1;
            ar[i][j+1]=-1;
            stp++;
        }
    }
}

void removeBarrier(int m, int n,int i, int j,int ar[][n]){
    int select=0;
    int pos[4]={0};
    int resp,temp;
    if (0<=i-1){
        if(ar[i-1][j]>0){
            pos[select]=4;
            select++;
        }
    }
    if (0<=j-1){
        if(ar[i][j-1]>0){
            pos[select]=3;
            select++;
        }
    }
    if (i+1<m){
        if(ar[i+1][j]>0){
            pos[select]=2;
            select++;
        }
    }
    if (j+1<n){
        if(ar[i][j+1]>0){
            pos[select]=1;
            select++;
        }
    }
    temp=rand()%select;
    resp=pos[temp];
    if (resp==1){
        ar[i][j+1]+=4;
        ar[i][j]=1;
    }
    else if (resp==2){
        ar[i+1][j]+=8;
        ar[i][j]=2;
    }
    else if (resp==3){
        ar[i][j-1]+=1;
        ar[i][j]=4;
    }
    else if (resp==4){
        ar[i-1][j]+=2;
        ar[i][j]=8;
    }
}

void setentrys(int m, int n,int ar[][n]){
    int ei,ej,si,sj,ig,p;
    ig=1;
    ei=ej=si=sj=0;
    if(m>1){
        ei=rand()% m;
        si=rand()% m;
    }
    if(n>1){
        ej=rand()% n;
        sj=rand()% n;
    }
    int pos[4]={0,ei,m-1,si};
    int alt[4]={ej,n-1,sj,0};
    p=rand()%4;
    ei=pos[p];
    ej=alt[p];
    while (ig){
        p=rand()%4;
        si=pos[p];
        sj=alt[p];
        if(ei*n+ej!=si*n+sj)
            ig=0;
    }
    Entry=ei*n+ej;
    Exit=si*n+sj;
    if (ej==n-1){
        ar[ei][ej]+=1;
    }
    else if (ei==m-1){
        ar[ei][ej]+=2;
    }
    else if (ej==0){
        ar[ei][ej]+=4;
    }
    else if (ei==0){
        ar[ei][ej]+=8;
    }

    if (sj==n-1){
        ar[si][sj]+=1;
    }
    else if (si==m-1){
        ar[si][sj]+=2;
    }
    else if (sj==0){
        ar[si][sj]+=4;
    }
    else if (si==0){
        ar[si][sj]+=8;
    }
}

void generate_maze(){
    int t,i,j,n,act;
    n=act=0;
    for(i =0; i<BaseF*2;i++){
        for(j =0; j<BaseC;j++){
            t=i>>1;
            act=ArBase[(BaseC*t+j)];
            if(!(i&1)){
                n++;
                if (act&8)
                    ArPant[n]=1;
            }else{
                if (act&4)
                    ArPant[n]=1;
                n++;
                ArPant[n]=1;
            }
            n++;
        }
        if((act&1)&&(i&1))
            ArPant[n]=1;
        n++;
    }
    for(j=0;j<BaseC;j++){
        n++;
        act=ArBase[(BaseF-1)*BaseC+j];
        if (act&2)
            ArPant[n]=1;
        n++;
    }
}

void desplegar(){
    for(int i = 0;i<((2*BaseF+1)*(2*BaseC+1));i++){
        if(i%(2*BaseC+1)==0)
            printf("\n");
        if(ArPant[i]==0)
            printf("XX");
        else
            printf("  ");
    }
}

void guardar_maze(char* archivo){
    FILE *fp;
    fp=fopen(archivo, "wb");
    fwrite((const void*)&BaseF, sizeof(int), 1, fp);
    fwrite((const void*)&BaseC, sizeof(int), 1, fp);
    fwrite((const void*)&Entry, sizeof(int), 1, fp);
    fwrite((const void*)&Exit, sizeof(int), 1, fp);
    fwrite(ArPant, sizeof(int), (2*BaseF+1)*(2*BaseC+1), fp);
    fclose(fp);
}

int abrir_maze(char* archivo){
    FILE *fp;
    if( access( archivo, F_OK ) != -1 ) {
		fp=fopen(archivo, "rb");
        fread(&BaseF, sizeof(int), 1, fp);
        fread(&BaseC, sizeof(int), 1, fp);
        fread(&Entry, sizeof(int), 1, fp);
        fread(&Exit, sizeof(int), 1, fp);
		fread(ArPant, sizeof(int), (2*BaseF+1)*(2*BaseC+1), fp);
		fclose(fp);
		return 1;
	} else
		return 0;
}


void etext(GtkSpinButton* entry,const gchar *text,gint length,gint *position, gpointer data){
	GtkEditable *editable = GTK_EDITABLE(entry);
	gchar *result = g_new (gchar,length);
	g_signal_handlers_block_by_func (G_OBJECT (editable), G_CALLBACK (etext),data);
	if (isdigit(text[*position])){
		result[*position]=text[*position];
		gtk_editable_insert_text (editable, result, length, position);
	}
	g_signal_handlers_unblock_by_func (G_OBJECT (editable),G_CALLBACK (etext),data);
	g_signal_stop_emission_by_name (G_OBJECT (editable), "insert_text");

	g_free (result);
}

void on_window_main_destroy()
{
    gtk_main_quit();
}

void on_SalirB_clicked()
{
	gtk_main_quit();
}

void on_GenerarB_clicked()
{
	if (!corriendo){
        corriendo=1;
        if(resolv(BaseF,BaseC,&ArBase))
            generate_maze();
	}
}

void on_ResolverB_clicked()
{
	if (!corriendo){
        corriendo=1;
        if(resolv(BaseF,BaseC,&ArBase))
            generate_maze();
	}
}

void on_AbrirB_clicked()
{
	if(!corriendo)
		gtk_dialog_run(GTK_DIALOG(DA));
}

void on_GuardarB_clicked()
{
	if(!corriendo)
		gtk_dialog_run(GTK_DIALOG(DG));
}

void on_CGAceptar_clicked(){
	eab=1;
	gtk_widget_hide(GTK_WIDGET(Confirm));
}

void on_CGCancelar_clicked(){
	gtk_widget_hide(GTK_WIDGET(Confirm));
}

void on_BGGuardar_clicked()
{
    archivo= gtk_file_chooser_get_current_name (GTK_FILE_CHOOSER(GUARDAR));
    Pistas();
    if( access( archivo, F_OK ) != -1 )
		gtk_dialog_run(Confirm);
	else
		eab=1;
	if (eab){
		guardar_maze(archivo);
		gtk_widget_hide(GTK_WIDGET(DG));
		eab=0;
	}
}

void on_BGCancelar_clicked()
{
	gtk_widget_hide(GTK_WIDGET(DG));
}

void on_BAAbrir_clicked()
{
	borrar();
	archivo=gtk_file_chooser_get_filename(ABRIR);
	if(abrir()==0)
		desplegar();
	gtk_widget_hide(GTK_WIDGET(DA));
}

void on_BACancelar_clicked()
{
	gtk_widget_hide(GTK_WIDGET(DA));
}

void on_msj_aceptar_clicked()
{
	gtk_widget_hide(GTK_WIDGET(mnsjResolv));
}

void Pasar(){
	gchar *text = g_new (gchar,5);
    text=gtk_entry_get_text(filas);
    BaseF=atoi(text);
    text=gtk_entry_get_text(columnas);
    BaseC=atoi(text);
}

int abrir(){
    if (abrir_maze(archivo))
        return 1;
    return 0;
}
