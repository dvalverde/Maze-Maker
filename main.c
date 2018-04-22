#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <unistd.h> //nanosleep(&ps, NULL); struct timespec ps;

int ArBase[4194304]={0};
int ArPant[16793604]={0};
int BaseF=1;
int BaseC=1;
int Entry=0;
int Exit=0;
int ready=0;
int maxd=0;

int SpwTrR[2796203]={0};
int SpwTrC[2796203]={0};
int stp;

char* archivo;

GtkDialog* mnsjResolv;
GtkDialog* Generar;
GtkDialog* Confirm;
GtkApplicationWindow* DAdial;
GtkDrawingArea* DrawArea;
GtkFileChooserDialog* DA;
GtkFileChooserDialog* DG;
GtkAdjustment *adjustmentf;
GtkAdjustment *adjustmentc;
GtkWidget* window;
GtkFileChooser* ABRIR;
GtkFileChooser* GUARDAR;
GtkSpinButton* filas;
GtkSpinButton* columnas;
GtkButton* GuardarB;

int corriendo=0;
int solucion=1;
int activo=0;
int lst;

int eab=0;

char *TRes;

static gint pos_x = 0;
static gint pos_y = 0;
static gint pos2_x = 0;
static gint pos2_y = 0;
static gint hg = 25; 
static gint wd = 25; 
static gint Thg = 25; 
static gint Twd = 25;
static int depth = 0;
static cairo_surface_t * completo=NULL;
static cairo_t *aux=NULL;

int guardar();
int abrir();
void Pasar();
void configspnbttn();

void set_basef();
void set_basec();
void calcular_d();
static void do_drawing();

int resolv(int m,int n,int ar[][n]);
void addFrontier(int m, int n,int i, int j,int ar[][n]);
void removeBarrier(int m, int n,int i, int j,int ar[][n]);
void setentrys(int m, int n,int ar[][n]); //n m > 1
void generate_maze();
void desplegar ();
int abrir_maze(char* archivo);
void guardar_maze(char* archivo);
static void clear_surface ();
static void do_bdrawing(cairo_t *cr);
static gboolean on_draw_event(GtkWidget *widget, cairo_t *cr, gpointer user_data);
static gboolean check_escape(GtkWidget *widget, GdkEventKey *event, gpointer data);


int main(int argc, char *argv[])
{
    srand(time(NULL));
    
    GtkBuilder      *builder;

    gtk_init(&argc, &argv);

    builder = gtk_builder_new();
    gtk_builder_add_from_file (builder, "inter.glade", NULL);

    window = GTK_WIDGET(gtk_builder_get_object(builder, "window_main"));

	TRes= g_new (gchar,20);

	mnsjResolv=GTK_DIALOG(gtk_builder_get_object(builder, "msj_Resolver"));
	DAdial=GTK_APPLICATION_WINDOW(gtk_builder_get_object(builder, "Maze_area"));
	DrawArea=GTK_DRAWING_AREA(gtk_builder_get_object(builder, "drawArea"));
	Generar=GTK_DIALOG(gtk_builder_get_object(builder, "msj_Generar"));
	Confirm=GTK_DIALOG(gtk_builder_get_object(builder, "Confirm"));
	filas=GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "spinbuttonFilas"));
    columnas=GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "spinbuttonColumnas"));
    DA=GTK_FILE_CHOOSER_DIALOG(gtk_builder_get_object(builder,"fileAbrir"));
    DG=GTK_FILE_CHOOSER_DIALOG(gtk_builder_get_object(builder,"fileGuardar"));
    GuardarB=GTK_BUTTON(gtk_builder_get_object(builder,"GuardarB"));
    ABRIR=GTK_FILE_CHOOSER(DA);
	GUARDAR=GTK_FILE_CHOOSER(DG);
    gtk_builder_connect_signals(builder, NULL);
    configspnbttn();
    gtk_widget_set_sensitive (GTK_WIDGET(GuardarB), FALSE);

    g_object_unref(builder);

    gtk_widget_show(window);
    gtk_main();
    /*
    BaseF=5;
	BaseC=6;//n m > 1
    if(resolv(BaseF,BaseC,&ArBase))
        generate_maze();
    guardar_maze("prueba.txt");
    desplegar();
    printf("\n \n");
    if (abrir_maze("prueba.txt"))
        desplegar();*/
    return 0;
}

int resolv(int m,int n,int ar[][n]){
	solucion=0;
    int spawn = rand() % (n*m);
    int mov;
    int i,j;
    stp=0;
    Entry=Exit=0;
    for(i=0;i<m;i++){
        for(j=0;j<n;j++)
            ar[i][j]=0;
    }
    if(n*m>1){
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
	}
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
    if(n*m==1){
        int sel[4]={1,2,4,8};
        ej=sel[p];
        while (ig){
            p=rand()%4;
            sj=sel[p];
            if(ej!=sj)
                ig=0;
        }
        ar[0][0]=ej+sj;
    }else{
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
}

void generate_maze(){
    int t,i,j,n,act;
    n=act=0;
    for(i =0; i<(BaseF*2+1)*(BaseC*2+1);i++){
			ArPant[i]=0;
	}
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
    solucion=1;
}

void desplegar(){
    int l=(2*BaseF+1)*(2*BaseC+1);
    int n=(2*BaseC+1);
    //int tam=10;
    for(int i = 0;i<l;i++){
        if(i%n==0)
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

void calcular_d(){
	Thg=Twd=hg=wd=25;
	pos2_x=pos_x=pos2_y=pos_y=0;
	depth=0;
	int max=BaseC;
	if (BaseC<BaseF)
		max=BaseF;
	maxd=0;
	while(20*maxd+15<(max*10+5)){
		maxd++;
	}
}

int range(int n,int w, int m){
	if (n<=0)
		return 0;
	else if (m<n+w)
		return m-w;
	else
		return n;
}

void zoomIn(){
	if (depth>0){
		depth--;
		hg=wd=20*depth+15;
		pos_x=pos_x+10;
		pos_y=pos_y+10;
	}
}

void zoomOut(){
	if (depth<maxd){
		depth++;
		hg=wd=20*depth+15;
		pos_x=range(pos_x-10,wd,Twd);
		pos_y=range(pos_y-10,hg,Thg);
	}else{
		int max=Twd;
		if(Twd<Thg)
			max=Thg;
		hg=wd=max;
		pos_x=0;
		pos_y=0;
	}
}

void desplazar(gdouble x, gdouble y,gint h, gint w){
	if (depth<maxd){
		pos_x=range(pos_x+(pos2_x - x),wd,Twd);
		pos_y=range(pos_y+(pos2_y - y),hg,Thg);
		pos2_x=x;
		pos2_y=y;
	}
}

void on_window_main_destroy()
{
    gtk_main_quit();
}

static gboolean check_escape(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	if (event->keyval == GDK_KEY_Escape) {
		gtk_widget_hide(GTK_WIDGET(DAdial));
		corriendo=0;
		activo=0;
		gtk_widget_set_sensitive (GTK_WIDGET(GuardarB), FALSE);
		return TRUE;
	}
	return FALSE;
}

void on_SalirB_clicked()
{
	gtk_main_quit();
}

void on_GenerarB_clicked()
{
	if(!activo){
		if (!corriendo){
			corriendo=1;
			activo=1;
			gtk_dialog_run(Generar);
		}
	}
}

void on_GenAceptar_clicked(){
	Pasar();
	if(resolv(BaseF,BaseC,&ArBase))
            generate_maze();
    calcular_d();
    if (completo)
		clear_surface ();
	gtk_widget_show(GTK_WIDGET(DAdial));
	
    gtk_widget_hide(GTK_WIDGET(Generar));
}

void on_GenCancelar_clicked(){
	corriendo=0;
	gtk_widget_hide(GTK_WIDGET(Generar));
}

void on_ResolverB_clicked()
{
	gtk_dialog_run(mnsjResolv);
}

void on_AbrirB_clicked()
{
	if(!corriendo && !activo){
		gtk_dialog_run(GTK_DIALOG(DA));
		activo=1;
		}
}

void on_GuardarB_clicked()
{
	if(corriendo&&solucion)
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
	if(!(activo)){
		archivo=gtk_file_chooser_get_filename(ABRIR);
		if(abrir()){
			calcular_d();
			if (completo)
				clear_surface ();
			gtk_widget_show(GTK_WIDGET(DAdial));
		}
		gtk_widget_hide(GTK_WIDGET(DA));
	}
}

void on_BACancelar_clicked()
{
	gtk_widget_hide(GTK_WIDGET(DA));
}

void on_msj_aceptar_clicked()
{
	gtk_widget_hide(GTK_WIDGET(mnsjResolv));
}

gboolean on_Maze_area_delete_event(GtkWidget *widget, GdkEvent *event, gpointer dat)
{
	gtk_widget_hide(widget);
	corriendo=0;
	activo=0;
	gtk_widget_set_sensitive (GTK_WIDGET(GuardarB), FALSE);
	return TRUE;
}

static void clear_surface ()
{
  if (completo)
    cairo_surface_destroy (completo);
  Thg = 10*BaseF+15; 
  Twd = 10*BaseC+15;
  int max=Thg;
	if(max<Twd)
		max=Twd;
  completo = gdk_window_create_similar_surface (gtk_widget_get_window (GTK_WIDGET(DrawArea)),CAIRO_CONTENT_COLOR,max,max);
  cairo_t *cr;
  cr = cairo_create (completo);
  cairo_set_source_rgb (cr, 1, 1, 1);
  cairo_paint (cr); 
  do_bdrawing(cr);
  cairo_destroy (cr);
  gtk_widget_set_sensitive (GTK_WIDGET(GuardarB), TRUE);
}

static gboolean configure_event_cb (GtkWidget *widget,GdkEventConfigure *event,gpointer data)
{
  if (completo)
    cairo_surface_destroy (completo);
  completo = gdk_window_create_similar_surface (gtk_widget_get_window (widget),CAIRO_CONTENT_COLOR,gtk_widget_get_allocated_width (widget),gtk_widget_get_allocated_height (widget));
  cairo_t *cr;
  cr = cairo_create (completo);
  cairo_set_source_rgb (cr, 1, 1, 1);
  cairo_paint (cr);
  cairo_destroy (cr);
  clear_surface ();
  
  return TRUE;
}

static gboolean scrollZ(GtkWidget * Widget,GdkEventScroll* event, gpointer data)
{
	if(event->direction==GDK_SCROLL_UP){
		zoomIn();
	}
	else if(event->direction==GDK_SCROLL_DOWN){
		zoomOut();
	}
	do_drawing();
	gtk_widget_queue_draw(Widget);
	return TRUE;
} 

static gboolean button_press_event_cb (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	if (event->button == GDK_BUTTON_PRIMARY){
		pos2_x=event->x;
		pos2_y=event->y;
   }
  return TRUE;
}

static gboolean mover (GtkWidget *widget, GdkEventMotion *event, gpointer data){
	if (event->state & GDK_BUTTON1_MASK){
		gint width, height;
		width=gtk_widget_get_allocated_width(widget);
		height=gtk_widget_get_allocated_height(widget);
		desplazar (event->x, event->y,height,width);
    }
	do_drawing();
	gtk_widget_queue_draw(widget);
	return TRUE;
}

static void do_drawing(){
	GtkWidget* widget=GTK_WIDGET(DrawArea);
	gint width, height;
	width=gtk_widget_get_allocated_width(widget);
	height=gtk_widget_get_allocated_height(widget);
	cairo_set_source_surface (aux, completo, 0, 0);

	GdkPixbuf *alt=gdk_pixbuf_get_from_surface(completo,pos_x,pos_y,wd, hg);
	GdkPixbuf *temp=gdk_pixbuf_scale_simple(alt, width, height, GDK_INTERP_BILINEAR);
	gdk_cairo_set_source_pixbuf(aux, temp, 0, 0);
	cairo_paint(aux);
	g_object_unref(alt);
	g_object_unref(temp);
}

static gboolean on_draw_event(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{ 
	if (aux==NULL)
		aux = cr;
	do_drawing();
	return TRUE;
}


static void do_bdrawing(cairo_t *cr){
	cairo_set_source_rgb(cr, 1, 1, 1);
	cairo_paint(cr);
	int l=(2*BaseF+1)*(2*BaseC+1);
    int n=(2*BaseC+1);
    int tam=5;
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_translate(cr, 5, 5);
    for(int i = 0;i<l;i++){
        if(i%n==0&&i!=0){
            cairo_translate(cr, -1*n*tam, tam);
        }
        if(ArPant[i]==0){
			cairo_rectangle(cr, 0, 0, tam, tam);
			cairo_fill(cr); 
		}
		cairo_translate(cr, tam, 0);
    }
}

void Pasar(){
	gint value=gtk_spin_button_get_value (filas);
    BaseF=value;
    value=gtk_spin_button_get_value(columnas);
    BaseC=value;
}

int abrir(){
    if (abrir_maze(archivo))
        return 1;
    return 0;
}

void configspnbttn(){
	adjustmentf = gtk_adjustment_new (0, 1, 2048, 1, 0, 0);
	adjustmentc = gtk_adjustment_new (0, 1, 2048, 1, 0, 0);
	gtk_spin_button_set_adjustment (filas,adjustmentf);
	gtk_spin_button_set_adjustment (columnas,adjustmentc);
	gtk_widget_add_events(GTK_WIDGET(DrawArea), GDK_SCROLL_MASK);
	gtk_widget_add_events(GTK_WIDGET(DrawArea), GDK_BUTTON_PRESS_MASK);
	gtk_widget_add_events(GTK_WIDGET(DrawArea), GDK_POINTER_MOTION_MASK);
	g_signal_connect(G_OBJECT(DrawArea), "draw",G_CALLBACK(on_draw_event), NULL);
    g_signal_connect(G_OBJECT(DAdial), "key_press_event", G_CALLBACK(check_escape), NULL);
    g_signal_connect(G_OBJECT(DAdial), "delete-event", G_CALLBACK(on_Maze_area_delete_event), NULL);
    g_signal_connect (G_OBJECT(DrawArea),"configure-event",G_CALLBACK (configure_event_cb), NULL);
    g_signal_connect(G_OBJECT(DrawArea), "scroll-event", G_CALLBACK(scrollZ), NULL);
    g_signal_connect(G_OBJECT(DrawArea), "motion-notify-event", G_CALLBACK(mover), NULL);
    g_signal_connect (G_OBJECT(DrawArea), "button-press-event", G_CALLBACK (button_press_event_cb), NULL);
	}

