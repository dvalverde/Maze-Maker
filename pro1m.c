#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <gtk/gtk.h>
#include <unistd.h>


int arregloNumeros[9][9]={0};
int pistas[9][9]={1};

GtkEntry* entradas[9][9];

char* archivo;

GtkDialog* mnsjResolv;
GtkDialog* Confirm;
GtkFileChooserDialog* DA;
GtkFileChooserDialog* DG;
GtkWidget* window;
GtkLabel* Solucionable;
GtkLabel* Tiempo;
GtkSpinner* Progreso;
GtkFileChooser* ABRIR;
GtkFileChooser* GUARDAR;
GtkWidget* grid;

struct entrMod{GtkEntry* entry; int dato;};
struct timespec ts;
struct timespec ps;

int valido (int n,int i, int j);

int threadIni;
int corriendo=0;
int solucion;
int validar=0;
int lst;

pthread_t cron_thread;
pthread_t sol_thread;

int eab=0;

char *TRes;

static gboolean updateTemp();
static gboolean finProg();
static gboolean setEntry(gpointer data);
static gboolean resetEntry(gpointer data);

void *solucionar(void *k);

void *cronometro(void *k);

void actualizarEntry(int i,int j,int n);

void refresh_i(int i);

int guardar();

int abrir();

int listo();

int Espacio();

void Pasar();

void Pistas();

void borrar();

void cargar(); 


int main(int argc, char *argv[])
{
    GtkBuilder      *builder;
    
    gtk_init(&argc, &argv);

    builder = gtk_builder_new();
    gtk_builder_add_from_file (builder, "inter.glade", NULL);
    
    window = GTK_WIDGET(gtk_builder_get_object(builder, "window_main"));
    
    ts.tv_sec = 0.015;
    ts.tv_nsec = 15000;
    ps.tv_sec = 0.005;
    ps.tv_nsec = 5000;
    int i=0;
    char entr[4] ;
    while (i<81){
		sprintf(entr,"e%d", i);
		entradas[i/9][i%9]=GTK_ENTRY(gtk_builder_get_object(builder, entr));
		i++;
	}
	TRes= g_new (gchar,20);
	
	mnsjResolv=GTK_DIALOG(gtk_builder_get_object(builder, "msj_Resolver"));
	Confirm=GTK_DIALOG(gtk_builder_get_object(builder, "Confirm"));
	
    DA=GTK_FILE_CHOOSER_DIALOG(gtk_builder_get_object(builder,"fileAbrir"));
    DG=GTK_FILE_CHOOSER_DIALOG(gtk_builder_get_object(builder,"fileGuardar"));
    ABRIR=GTK_FILE_CHOOSER(DA);
	GUARDAR=GTK_FILE_CHOOSER(DG);;
    
    Solucionable=GTK_LABEL(gtk_builder_get_object(builder, "Solucionable"));
    
    gtk_builder_connect_signals(builder, NULL);

    g_object_unref(builder);
    
    threadIni=1;
    lst=0;
    if(pthread_create(&cron_thread, NULL, cronometro,&corriendo)) {
		fprintf(stderr, "Error creating thread cron\n");
	}
	if(pthread_create(&sol_thread, NULL, solucionar,&corriendo)) {
		fprintf(stderr, "Error creating thread sol\n");
	}

    gtk_widget_show(window);
    gtk_main();
    threadIni=0;
    
    if(pthread_join(sol_thread, NULL)) {
		fprintf(stderr, "Error joining thread sol\n");
	}
	if(pthread_join(cron_thread, NULL)) {
		fprintf(stderr, "Error joining thread cron\n");
	}
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

void on_ResolverB_clicked()
{
	if (!corriendo){
		gtk_label_set_text (Tiempo,"0");
		Pasar();
		if (!listo()&&validar)
			gtk_dialog_run(mnsjResolv);
		else{
			if (Espacio()){
				gtk_label_set_text (Solucionable,"Buscando");
				gtk_spinner_start (Progreso);
				corriendo=1;
			}
		}
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
		guardar();
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
		cargar();
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

static gboolean updateTemp(){
	gtk_label_set_text (Tiempo,TRes);
	gtk_widget_queue_draw (GTK_WIDGET(Tiempo));
	/*while(gtk_events_pending())
		gtk_main_iteration();*/
	return G_SOURCE_REMOVE;
}

static gboolean finProg(){
	gtk_spinner_stop (Progreso);
	if(solucion){
		gtk_label_set_text (Solucionable,"Si");
		}
	else{
		gtk_label_set_text (Solucionable,"No");
		}
		nanosleep(&ts, NULL);
		refresh_i(0);
	return G_SOURCE_REMOVE;
}

static gboolean setEntry(gpointer data){
	struct entrMod *ent1;
	ent1=data;
	gchar *NRes = g_new (gchar,1);
	sprintf(NRes,"%d", ent1->dato);
	gtk_entry_set_text (GTK_ENTRY(ent1->entry),NRes);
	gtk_widget_queue_draw (GTK_WIDGET(ent1->entry));
	while(gtk_events_pending())
		gtk_main_iteration();
	return G_SOURCE_REMOVE;
}

static gboolean resetEntry(gpointer data){
	struct entrMod *ent1;
	ent1=data;
	gtk_entry_set_text (GTK_ENTRY(ent1->entry),"");
	gtk_widget_queue_draw (GTK_WIDGET(ent1->entry));
	while(gtk_events_pending())
		gtk_main_iteration();
	return G_SOURCE_REMOVE;
}

void *cronometro(void *k){
	int tiemp;
	int i=lst;
	while(threadIni){
		if(corriendo){
			clock_t tpi,tpn;
			tpi = clock();
			while(corriendo){
				tpn= clock() - tpi;
				tiemp=((int)tpn)/(CLOCKS_PER_SEC/1000000);
				sprintf(TRes,"%d", tiemp);
				g_main_context_invoke (NULL, updateTemp, Tiempo);
				nanosleep(&ts, NULL);
				if (i!= lst)
					refresh_i(i);
			}
			g_main_context_invoke (NULL,finProg,Tiempo);
		}
	}
	return NULL;
}

void *solucionar(void *k){
	while(threadIni){
		if(corriendo){
			int ultima;
			int anter[81];
			lst=0;
			int i=0;
			int j=0;
			int term=0;
			while (i<9&&j<9 && arregloNumeros[i][j]!=0){
				j++;
				if (j>8){
					i++;
					j=0;
				}
			}
			arregloNumeros[i][j]=1;
			anter[0]=i*9+j;
			ultima =0;
			while (!term){
				nanosleep(&ts, NULL);
				while ((valido(arregloNumeros[i][j],i,j)==0)&&(arregloNumeros[i][j]<=9))
					arregloNumeros[i][j]++;
				actualizarEntry(i,j,arregloNumeros[i][j]);
				nanosleep(&ts, NULL);
				if (arregloNumeros[i][j]<=9){
					while (i<9&&j<9 && arregloNumeros[i][j]!=0){
						j++;
						if (j>8){
							i++;
							j=0;
						}
					}
					if (i<9&&j<9){
						ultima++;
						anter[ultima]=i*9+j;
						arregloNumeros[i][j]=1;
					}
					else{
						term=1;
						solucion=1;//solucion
					}
				}else{
					arregloNumeros[i][j]=0;
					if (ultima>0){
						ultima--;
						i=anter[ultima]/9;
						j=anter[ultima]%9;
						arregloNumeros[i][j]++;
					}
					else{
						term=1;
						solucion=0;//no solucion
					}
				}
				nanosleep(&ts, NULL);
				if(i>1 && i>lst)
					lst=i;
			}
			corriendo=0;
		}
	}
    return NULL;
}

void actualizarEntry(int i,int j,int n){
	if(i<10 && j<10 && i>=0 && j>=0){
		struct entrMod ent1;
		ent1.entry=entradas[i][j];
		if (0<n && n<10){
			ent1.dato=n;
			g_main_context_invoke (NULL,setEntry,&ent1);
		}else{
			ent1.dato=0;
			g_main_context_invoke (NULL,resetEntry,&ent1);
		}
	}
}

void cargar(){
	int i=0;
	int j;
	while (i<9){
		j=0;
		while(j<9){
			if(pistas[i][j]!=0)
				actualizarEntry(i,j,pistas[i][j]);
			j++;
		}
		i++;
	}
}

void refresh_i(int i){
	int n=0;
	if (corriendo){
		while (n<9){
			if(i>0 && pistas[i-1][n]!=0)
				actualizarEntry(i-1,n,arregloNumeros[i-1][n]);
			nanosleep(&ts, NULL);
			n++;
		}
	}else{
		i=0;
		while (i<9){
			actualizarEntry(i,n,arregloNumeros[i][n]);
			n++;
			nanosleep(&ps, NULL);
			if(n>8){
				n=0;
				i++;
			}
		}
	}
}

void borrar(){
	int i=0;
	int j;
	while (i<9){
		j=0;
		while(j<9){
			pistas[i][j]=0;
			actualizarEntry(i,j,0);
			j++;
		}
		i++;
	}
	gtk_label_set_text (Tiempo,"0");
	gtk_label_set_text (Solucionable,"?");
}

void Pasar(){
	int n=0;
	GtkEntry* act;
	gchar *text = g_new (gchar,1);
	while (n<81){
		act=entradas[n/9][n%9];
		text=gtk_entry_get_text(act);
		if(text[0]=='1')
			pistas[n/9][n%9]=arregloNumeros[n/9][n%9]=1;
		else if (text[0]=='2')
			pistas[n/9][n%9]=arregloNumeros[n/9][n%9]=2;
		else if (text[0]=='3')
			pistas[n/9][n%9]=arregloNumeros[n/9][n%9]=3;
		else if (text[0]=='4')
			pistas[n/9][n%9]=arregloNumeros[n/9][n%9]=4;
		else if (text[0]=='5')
			pistas[n/9][n%9]=arregloNumeros[n/9][n%9]=5;
		else if (text[0]=='6')
			pistas[n/9][n%9]=arregloNumeros[n/9][n%9]=6;
		else if (text[0]=='7')
			pistas[n/9][n%9]=arregloNumeros[n/9][n%9]=7;
		else if (text[0]=='8')
			pistas[n/9][n%9]=arregloNumeros[n/9][n%9]=8;
		else if (text[0]=='9')
			pistas[n/9][n%9]=arregloNumeros[n/9][n%9]=9;
		else
			pistas[n/9][n%9]=arregloNumeros[n/9][n%9]=0;
		n++;
	}
	return;
}

int Espacio(){
	int n=0;
	while (n<81){
		if(pistas[n/9][n%9]==0)
			return 1;
		n++;
	}
	return 0;
}

void Pistas(){
	int n=0;
	GtkEntry* act;
	gchar *text = g_new (gchar,1);
	while (n<81){
		act=entradas[n/9][n%9];
		text=gtk_entry_get_text(act);
		if(text[0]=='1')
			pistas[n/9][n%9]=1;
		else if (text[0]=='2')
			pistas[n/9][n%9]=2;
		else if (text[0]=='3')
			pistas[n/9][n%9]=3;
		else if (text[0]=='4')
			pistas[n/9][n%9]=4;
		else if (text[0]=='5')
			pistas[n/9][n%9]=5;
		else if (text[0]=='6')
			pistas[n/9][n%9]=6;
		else if (text[0]=='7')
			pistas[n/9][n%9]=7;
		else if (text[0]=='8')
			pistas[n/9][n%9]=8;
		else if (text[0]=='9')
			pistas[n/9][n%9]=9;
		else
			pistas[n/9][n%9]=0;
		n++;
	}
	return;
}

int valido (int n,int i, int j){
    int x=0;
    while (x<9){
            if((arregloNumeros[i][x]==n && x!=j) || (arregloNumeros[x][j]==n && x!=i))
                return 0;//invalido
            x++;
    }
    x=0;
    int y=i/3;
    int z=j/3;
    y*=3;
    z*=3;
    int a,b;
    while(x<9){
            a=x/3+y;
            b=x%3+z;
            if((arregloNumeros[a][b]==n && a!=i && b!=j)){
                return 0;}//invalido
            x++;
    }
    return 1;
}

int guardar(){
    FILE *fp;
    fp=fopen(archivo, "wb");
    int x[81]={0};
    int i=0;
	int j;
	while (i<9){
		j=0;
		while(j<9){
			x[i*9+j]=pistas[i][j];
			j++;
		}
		i++;
	}
    fwrite(x, sizeof(int), sizeof(x)/sizeof(int), fp);
    fclose(fp);
    return 0;
}

int abrir(){
    FILE *fp;
    if( access( archivo, F_OK ) != -1 ) {
		fp=fopen(archivo, "rb");
		int x[81];
		fread(x, sizeof(int), sizeof(x)/sizeof(int), fp);
		int i=0;
		int j;
		while (i<9){
			j=0;
			while(j<9){
				pistas[i][j]=arregloNumeros[i][j]=x[i*9+j];
				j++;
			}
			i++;
		}
		fclose(fp);
		return 0;
	} else 
		return 0;
}

int listo(){
    int n=0;
    int i,j,act;
    while (n<81){
        i=n/9;
        j=n%9;
        act=arregloNumeros[i][j];
        if ((act!=0)&&(valido(act,i,j)==0))
            return 0;
        n++;
    }
    return 1;
}



