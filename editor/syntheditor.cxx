/** Minicomputer
 * industrial grade digital synthesizer
 * editorsoftware
 * Copyright 2007, 2008 Malte Steiner
 * This file is part of Minicomputer, which is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Minicomputer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "syntheditor.h"
#include "communicate.h"
#include "utility.h"
#include "i18n.h"
static Fl_RGB_Image image_miniMini(idata_miniMini, 191, 99, 3, 0);
// gcc -o synthEditor2 syntheditor.cxx -lfltk -llo
Fl_Widget *Knob[8][_PARACOUNT];
Fl_Choice *auswahl[8][17];
Fl_Value_Input *miniDisplay[8][13];
Fl_Widget *tab[9];
Fl_Input *schoice[8];
Mw_Roller *Rollers[8];
Mw_Roller *multiRoller;
Fl_Tabs *tabs;
Fl_Button *lm, *sm;
Fl_Value_Input *paramon;
Fl_Value_Output *memDisplay[8], *multiDisplay;
Fl_Input *Multichoice;

int currentParameter = 0;

unsigned int currentsound = 0, currentmulti = 0;
// unsigned int multi[128][8];
// string multiname[128];
// Fl_Chart * EG[7];
static void choicecallback(Fl_Widget *o, void *)
{
    sendChoice(currentsound,
               ((Fl_Choice *)o)->argument(), ((Fl_Choice *)o)->value());
}
/* // not good:
static void changemulti(Fl_Widget* o, void*)
{
    if (Multichoice != NULL)
    {
        int t = Multichoice->menubutton()->value();
        if ((t!=currentmulti) && (t>-1) && (t<128))
        {
            // ok, we are on somewhere other multi so we need to copy the settings
        //	for (int i=0;i<8;++i)
        //	{
        //		Speicher.multis[t].sound[i] = Speicher.multis[currentmulti].sound[i];
        //	}
            currentmulti = t;
        }
    }
    else
        printf("problems with the multichoice widgets!\n");
}
*/
/**
 * callback when another tab is chosen
 * @param Fl_Widget the calling widget
 * @param defined by FLTK but not used
 */
static void tabcallback(Fl_Widget *o, void *)
{
    Fl::lock();
    // int* g;
    // g=(int*)e;

    // currentsound=*g;
    // printf("sound %i value  xtab
    // %i\n",(int)((Fl_Group*)o)->argument(),*g);//currentsound); fflush(stdout);
    Fl_Widget *e = ((Fl_Tabs *)o)->value();
    if (e == tab[8]) {
        if (multiDisplay != NULL)
            multiDisplay->hide();
        else
            printf("there seems to be something wrong with multiroller widget");

        if (multiRoller != NULL)
            multiRoller->hide();
        else
            printf("there seems to be something wrong with multiroller widget");

        if (Multichoice != NULL)
            Multichoice->hide();
        else
            printf("there seems to be something wrong with multichoice widget");
        if (sm != NULL)
            sm->hide();
        else
            printf("there seems to be something wrong with storemultibutton "
                   "widget");

        if (lm != NULL)
            lm->hide();
        else
            printf("there seems to be something wrong with loadmultibutton "
                   "widget");


        if (paramon != NULL)
            paramon->hide();
        else
            printf("there seems to be something wrong with paramon widget");
    }
    else {
        if (multiDisplay != NULL)
            multiDisplay->show();
        else
            printf(
                "there seems to be something wrong with multiDisplay widget");

        if (multiRoller != NULL)
            multiRoller->show();
        else
            printf("there seems to be something wrong with multiroller widget");

        if (Multichoice != NULL)
            Multichoice->show();
        else
            printf("there seems to be something wrong with multichoice widget");
        if (sm != NULL)
            sm->show();
        else
            printf("there seems to be something wrong with storemultibutton "
                   "widget");

        if (lm != NULL)
            lm->show();
        else
            printf("there seems to be something wrong with loadmultibutton "
                   "widget");

        if (paramon != NULL)
            paramon->show();
        else
            printf("there seems to be something wrong with paramon widget");
        for (int i = 0; i < 8; ++i) {
            if (e == tab[i]) {
                currentsound = i;
                break;
            }
        }
        // else currentsound=1;
#ifdef _DEBUG
        printf("sound %i\n", currentsound);
        fflush(stdout);
#endif
    }  // end of else

    Fl::awake();
    Fl::unlock();
}
/**
 * main callback, called whenever a parameter has changed
 * @param Fl_Widget the calling widget
 * @param defined by FLTK but not used
 */
static void callback(Fl_Widget *o, void *)
{
    Fl::lock();
    if (o != NULL) {
        currentParameter = ((Fl_Valuator *)o)->argument();

        // show only parameter on finetune when its not a frequency
        switch (currentParameter) {
        case 1:
        case 3:
        case 16:
        case 18:
        case 30:
        case 33:
        case 40:
        case 43:
        case 50:
        case 53:
        case 90:
            break;  // do nothin
        default:
            paramon->value(((Fl_Valuator *)o)->value());
            break;
        }
        // float wert=-1024;
        //(Fl_Valuator*)o)->
        // printf("%li : %g     \r", ((Fl_Valuator*)o)->argument(),((Fl_Valuator*)o)->value());
        /*
      if (strcmp(o->label(),"f1")==0)
      {
          lo_send(t, "/f1", "f",((Fl_Valuator*)o)->value());
      }
      if (strcmp(o->label(),"f2")==0)
      {
          lo_send(t, "/f2", "f",((Fl_Valuator*)o)->value());
      }
      if (strcmp(o->label(),"f3")==0)
      {
          lo_send(t, "/f3", "f",((Fl_Valuator*)o)->value());
      }*/
        /*if (((Fl_Valuator*)o)->argument()!=10)
            lo_send(t, "/Minicomputer",
        "if",((Fl_Valuator*)o)->argument(),((Fl_Valuator*)o)->value()); else if
        (((Fl_Valuator*)o)->value()!=0) lo_send(t, "/Minicomputer",
        "if",10,1000.f); else lo_send(t, "/Minicomputer", "if",10,0.f);
        */

        // now actually process parameter
        switch (currentParameter) {
        case 256: {
            sendParameter(currentsound, 0, 0);
            break;
        }
        case 1: {
            sendParameter(currentsound,
                          ((Fl_Valuator *)o)->argument(), ((Fl_Valuator *)o)->value());
            miniDisplay[currentsound][0]->value(
                ((Fl_Valuator *)Knob[currentsound][1])->value());
#ifdef _DEBUG
            printf("%li : %g     \r", ((Fl_Valuator *)o)->argument(),
                   ((Fl_Valuator *)o)->value());
#endif
            break;
        }
        case 2: {
            if (((Fl_Light_Button *)o)->value() == 0) {
                Knob[currentsound][1]->deactivate();
                Knob[currentsound][3]->activate();
                miniDisplay[currentsound][1]->activate();
                miniDisplay[currentsound][0]->deactivate();
                sendParameter(currentsound,
                            ((Fl_Light_Button *)o)->argument(), 0.f);
            }
            else {
                Knob[currentsound][3]->deactivate();
                Knob[currentsound][1]->activate();
                miniDisplay[currentsound][0]->activate();
                miniDisplay[currentsound][1]->deactivate();
                sendParameter(currentsound,
                            ((Fl_Light_Button *)o)->argument(), 1.f);
            }
#ifdef _DEBUG
            printf("%li : %i     \r", ((Fl_Light_Button *)o)->argument(),
                   ((Fl_Light_Button *)o)->value());
#endif
            break;
        }
        case 17: {
            if (((Fl_Light_Button *)o)->value() == 0) {
                Knob[currentsound][16]->deactivate();
                Knob[currentsound][18]->activate();
                miniDisplay[currentsound][3]->activate();
                miniDisplay[currentsound][2]->deactivate();
                sendParameter(currentsound,
                            ((Fl_Light_Button *)o)->argument(), 0.f);
            }
            else {
                Knob[currentsound][18]->deactivate();
                Knob[currentsound][16]->activate();
                miniDisplay[currentsound][2]->activate();
                miniDisplay[currentsound][3]->deactivate();
                sendParameter(currentsound,
                            ((Fl_Light_Button *)o)->argument(), 1.f);
            }
#ifdef _DEBUG
            printf("%li : %i     \r", ((Fl_Light_Button *)o)->argument(),
                   ((Fl_Light_Button *)o)->value());
#endif
            break;
        }
        case 3: {
            float f = ((Fl_Positioner *)o)->xvalue() + ((Fl_Positioner *)o)->yvalue();
            sendParameter(currentsound,
                        ((Fl_Positioner *)o)->argument(), f);
            miniDisplay[currentsound][1]->value(f);
#ifdef _DEBUG
            printf("%li : %g     \r", ((Fl_Positioner *)o)->argument(), f);
#endif
            break;
        }
        case 16: {
            /*float f = ((Fl_Positioner*)o)->xvalue() +
            ((Fl_Positioner*)o)->yvalue(); lo_send(t, "/Minicomputer",
            "if",((Fl_Positioner*)o)->argument(),f); miniDisplay[2]->value( f);
            printf("%li : %g     \r", ((Fl_Positioner*)o)->argument(),f);*/
            sendParameter(currentsound,
                        ((Fl_Valuator *)o)->argument(), ((Fl_Valuator *)o)->value());
            miniDisplay[currentsound][2]->value(((Fl_Valuator *)o)->value());  // Knob[16])->value() );
#ifdef _DEBUG
            printf("%li : %g     \r", ((Fl_Valuator *)o)->argument(),
                   ((Fl_Valuator *)o)->value());
#endif
            break;
        }
        case 18: {
            float f = ((Fl_Positioner *)o)->xvalue() + ((Fl_Positioner *)o)->yvalue();
            sendParameter(currentsound,
                        ((Fl_Positioner *)o)->argument(), f);
            miniDisplay[currentsound][3]->value(f);
#ifdef _DEBUG
            printf("%li : %g     \r", ((Fl_Positioner *)o)->argument(), f);
#endif
            /*lo_send(t, "/Minicomputer", "if",((Fl_Valuator*)o)->argument(),((Fl_Valuator*)o)->value());
                    miniDisplay[3]->value( ((Fl_Valuator* )Knob[18])->value() );
                    printf("%li : %g     \r", ((Fl_Valuator*)o)->argument(),((Fl_Valuator*)o)->value());*/
            break;
        }
        case 4:  // boost button
        case 15: {
            if (((Fl_Light_Button *)o)->value() == 0) {
                sendParameter(currentsound,
                            ((Fl_Light_Button *)o)->argument(), 1.f);
            }
            else {
                sendParameter(currentsound,
                            ((Fl_Light_Button *)o)->argument(), 100.f);
            }
#ifdef _DEBUG
            printf("%li : %i     \r", ((Fl_Light_Button *)o)->argument(),
                   ((Fl_Light_Button *)o)->value());
#endif
            break;
        }
        // the repeat buttons of the mod egs + sync button
        case 64:
        case 69:
        case 74:
        case 79:
        case 84:
        case 89:
        case 115: {
            if (((Fl_Light_Button *)o)->value() == 0) {
                sendParameter(currentsound,
                            ((Fl_Light_Button *)o)->argument(), 0.f);
            }
            else {
                sendParameter(currentsound,
                            ((Fl_Light_Button *)o)->argument(), 1.f);
            }
#ifdef _DEBUG
            printf("%li : %i     \r", ((Fl_Light_Button *)o)->argument(),
                   ((Fl_Light_Button *)o)->value());
#endif
            break;
        }
        /*
        case 0:
        case 6:
        case 8:
        case 10:
        case 12:
        case 20:
        case 22:
        case 24:
        case 26:
        case 27:
        case 39:
        case 49:
        case 91:

        {
            lo_send(t, "/Minicomputer", "if",((Fl_Choice*)o)->argument(),(float)((Fl_Choice*)o)->value());
            printf("%li : %i     \r", ((Fl_Choice*)o)->argument(),((Fl_Choice*)o)->value());
            break;
        }*/
        case 60:
        case 61:
        case 63:
        case 65:
        case 66:
        case 68:

        case 70:
        case 71:
        case 73:
        case 75:
        case 76:
        case 78:

        case 80:
        case 81:
        case 83:
        case 85:
        case 86:
        case 88:


        case 102:
        case 103:
        case 105: {
            float tr = (((Fl_Valuator *)o)->value());  /// 200.f;//exp(((Fl_Valuator*)o)->value())/200.f;
            tr *= tr * tr / 2.f;  // tr * tr*20.f;//48000.0f;//trtr*tr/2;
            sendParameter(currentsound,
                        ((Fl_Valuator *)o)->argument(), tr);
#ifdef _DEBUG
            printf("eg %li : %g     \r", ((Fl_Valuator *)o)->argument(), tr);
#endif
            break;
        }

        //************************************ filter cuts *****************************
        case 30: {
            float f = ((Fl_Positioner *)o)->xvalue() + ((Fl_Positioner *)o)->yvalue();
            sendParameter(currentsound,
                        ((Fl_Positioner *)o)->argument(), f);
#ifdef _DEBUG
            printf("%li : %g     \r", ((Fl_Positioner *)o)->argument(), f);
#endif
            miniDisplay[currentsound][4]->value(f);
            break;
        }
        case 33: {
            float f = ((Fl_Positioner *)o)->xvalue() + ((Fl_Positioner *)o)->yvalue();
            sendParameter(currentsound,
                        ((Fl_Positioner *)o)->argument(), f);
#ifdef _DEBUG
            printf("%li : %g     \r", ((Fl_Positioner *)o)->argument(), f);
#endif
            miniDisplay[currentsound][5]->value(f);
            break;
        }
        case 40: {
            float f = ((Fl_Positioner *)o)->xvalue() + ((Fl_Positioner *)o)->yvalue();
            sendParameter(currentsound,
                        ((Fl_Positioner *)o)->argument(), f);
#ifdef _DEBUG
            printf("%li : %g     \r", ((Fl_Positioner *)o)->argument(), f);
#endif
            miniDisplay[currentsound][6]->value(f);
            break;
        }
        case 43: {
            float f = ((Fl_Positioner *)o)->xvalue() + ((Fl_Positioner *)o)->yvalue();
            sendParameter(currentsound,
                        ((Fl_Positioner *)o)->argument(), f);
#ifdef _DEBUG
            printf("%li : %g     \r", ((Fl_Positioner *)o)->argument(), f);
#endif
            miniDisplay[currentsound][7]->value(f);
            break;
        }
        case 50: {
            float f = 0;
            int Argument = 0;

            // if (!isFine)
            //{
            f = ((Fl_Positioner *)o)->xvalue() + ((Fl_Positioner *)o)->yvalue();
            Argument = ((Fl_Positioner *)o)->argument();
            //}
            // else
            //{
            //	f=(((Fl_Valuator*)o)->value());
            //	isFine = false;
            //	Argument=((Fl_Valuator*)o)->argument();
            //}*/
            sendParameter(currentsound, Argument, f);
#ifdef _DEBUG
            printf("%i : %g     \r", Argument, f);
#endif
            // printf(",,do it\n");
            fflush(stdout);
            miniDisplay[currentsound][8]->value(f);

            break;
        }
        case 53: {
            float f = ((Fl_Positioner *)o)->xvalue() + ((Fl_Positioner *)o)->yvalue();
            sendParameter(currentsound,
                        ((Fl_Positioner *)o)->argument(), f);
#ifdef _DEBUG
            printf("%li : %g     \r", ((Fl_Positioner *)o)->argument(), f);
#endif
            miniDisplay[currentsound][9]->value(f);
            break;
        }
        case 90: {
            float f = ((Fl_Positioner *)o)->xvalue() + ((Fl_Positioner *)o)->yvalue();
            sendParameter(currentsound,
                        ((Fl_Positioner *)o)->argument(), f);
#ifdef _DEBUG
            printf("%li : %g     \r", ((Fl_Positioner *)o)->argument(), f);
#endif
            miniDisplay[currentsound][10]->value(f);
            break;
        }
        default: {
            sendParameter(currentsound,
                        ((Fl_Valuator *)o)->argument(), ((Fl_Valuator *)o)->value());
#ifdef _DEBUG
            printf("%li : %g     \r", ((Fl_Valuator *)o)->argument(),
                   ((Fl_Valuator *)o)->value());
#endif
            break;
        }
        }

#ifdef _DEBUG
        fflush(stdout);
#endif
    }  // end of o != NULL

    Fl::awake();
    Fl::unlock();
}  // end of callback

/**
 * copybutton callback, called whenever the user wants to copy filterparameters
 * @param Fl_Widget the calling widget
 * @param defined by FLTK but not used
 */
/*
static void copycallback(Fl_Widget* o, void*) {

    int filter = ((Fl_Valuator*)o)->argument();// what to copy there
    switch (filter)
    {
    case 21:
    {

        ((Fl_Valuator* )Knob[currentsound][33])->value(	((Fl_Valuator*
)Knob[currentsound][30])->value()); callback(Knob[currentsound][33],NULL);
        ((Fl_Valuator* )Knob[currentsound][34])->value(	((Fl_Valuator*
)Knob[currentsound][31])->value()); callback(Knob[currentsound][34],NULL);
        ((Fl_Valuator* )Knob[currentsound][35])->value(	((Fl_Valuator*
)Knob[currentsound][32])->value()); callback(Knob[currentsound][35],NULL);
    }
    break;
    }
}*/
/**
 * callback for finetuning the current parameter
 * @param Fl_Widget the calling widget
 * @param defined by FLTK but not used
 */
static void finetune(Fl_Widget *o, void *)
{
    if (currentParameter < _PARACOUNT)  // range check
    {
        switch (currentParameter) {
        case 1:
        case 3:
        case 16:
        case 18:
        case 30:
        case 33:
        case 40:
        case 43:
        case 50:
        case 53:
        case 90:
            break;  // do nothin
        default:
            Fl::lock();
            ((Fl_Valuator *)Knob[currentsound][currentParameter])
                ->value(((Fl_Valuator *)o)->value());
            callback(Knob[currentsound][currentParameter], NULL);
            Fl::awake();
            Fl::unlock();
            break;
        }
    }
}
/*
static void lfoCallback(Fl_Widget* o, void*)
{
    int Faktor = (int)((Fl_Valuator* )o)->value();
    float Rem = ((Fl_Valuator* )o)->value()-Faktor;
    int Argument = ((Fl_Valuator* )o)->argument();
    ((Fl_Positioner* )Knob[currentsound][Argument])->xvalue(Faktor);
    ((Fl_Positioner* )Knob[currentsound][Argument])->yvalue(Rem);
    callback(Knob[currentsound][Argument],NULL);
}
*/
/** callback when a cutoff has changed
 * these following two callbacks are specialized
 * for the Positioner widget which is 2 dimensional
 * @param Fl_Widget the calling widget
 * @param defined by FLTK but not used
 */
static void cutoffCallback(Fl_Widget *o, void *)
{
    Fl::lock();
    int Faktor = ((int)(((Fl_Valuator *)o)->value() / 1000) * 1000);
    float Rem = ((Fl_Valuator *)o)->value() - Faktor;
    int Argument = ((Fl_Valuator *)o)->argument();
    ((Fl_Positioner *)Knob[currentsound][Argument])->xvalue(Faktor);
    ((Fl_Positioner *)Knob[currentsound][Argument])->yvalue(Rem);
    callback(Knob[currentsound][Argument], NULL);
    Fl::awake();
    Fl::unlock();
}
/** callback for frequency positioners in the oscillators
 * which are to be treated a bit different
 *
 * @param Fl_Widget the calling widget
 * @param defined by FLTK but not used
 */
static void tuneCallback(Fl_Widget *o, void *)
{
    Fl::lock();
    int Faktor = (int)((Fl_Valuator *)o)->value();
    float Rem = ((Fl_Valuator *)o)->value() - Faktor;
    int Argument = ((Fl_Valuator *)o)->argument();
    ((Fl_Positioner *)Knob[currentsound][Argument])->xvalue(Faktor);
    ((Fl_Positioner *)Knob[currentsound][Argument])->yvalue(Rem);
    callback(Knob[currentsound][Argument], NULL);
    Fl::awake();
    Fl::unlock();
}

static void rollerCallback(Fl_Widget *o, void *)
{
    Fl::lock();
    int Faktor = (int)((Fl_Valuator *)o)->value();
    schoice[currentsound]->value((Speicher.getName(0, Faktor)).c_str());  // Speicher.multis[currentmulti].sound[currentsound]);// set gui
    schoice[currentsound]->position(0);

    memDisplay[currentsound]->value(Faktor);  // set gui
    Fl::awake();
    Fl::unlock();
}
/*
static void chooseCallback(Fl_Widget* o, void*)
{
//	int Faktor = (int)((Fl_Valuator* )o)->value();
    Rollers[currentsound]->value(schoice[currentsound]->menubutton()->value());// set gui
    memDisplay[currentsound]->value(schoice[currentsound]->menubutton()->value());// set gui
}*/
static void multiRollerCallback(Fl_Widget *o, void *)
{
    Fl::lock();
    int Faktor = (int)((Fl_Valuator *)o)->value();
    Multichoice->value(Speicher.multis[Faktor].name);  // set gui
    Multichoice->position(0);  // put cursor in the beginning, otherwise the begin of the string might be invisible
    multiDisplay->value(Faktor);
    Fl::awake();
    Fl::unlock();
}
/*
static void chooseMultiCallback(Fl_Widget* o, void*)
{
    multiRoller->value(Multichoice->menubutton()->value());// set gui
}*/
/**
 * callback from the export filedialog
 * do the actual export of current single sound
 */
/*
static void exportSound(Fl_File_Chooser *w, void *userdata)
{
        printf("to %d\n",w->shown());
        fflush(stdout);
    if ((w->shown() !=1) && (w->value() != NULL))
    {
        printf("export to %s\n",w->value());
        fflush(stdout);
    }
}*/
/** callback when export button was pressed, shows a filedialog
 */
static void exportPressed(Fl_Widget *o, void *)
{
    std::string warn = astrprintf(_("export %s:"), schoice[currentsound]->value());
    Fl_File_Chooser *fc = new Fl_File_Chooser(".", _("TEXT Files (*.txt)\t"),
                                              Fl_File_Chooser::CREATE, warn.c_str());
    // fc->callback(exportSound); // not practical, is called to often
    fc->textsize(9);
    fc->show();
    while (fc->shown())
        Fl::wait();  // block until choice is done
    if ((fc->value() != NULL)) {
#ifdef _DEBUG
        printf("export to %s\n", fc->value());
        fflush(stdout);
#endif
        Fl::lock();
        fl_cursor(FL_CURSOR_WAIT, FL_WHITE, FL_BLACK);
        Fl::check();

        Speicher.exportSound(fc->value(), (unsigned int)memDisplay[currentsound]->value());

        fl_cursor(FL_CURSOR_DEFAULT, FL_WHITE, FL_BLACK);
        Fl::check();
        Fl::awake();
        Fl::unlock();
    }
}
/** callback when import button was pressed, shows a filedialog
 */
static void importPressed(Fl_Widget *o, void *)
{
    std::string warn = astrprintf(_("overwrite %s:"), schoice[currentsound]->value());
    Fl_File_Chooser *fc = new Fl_File_Chooser(".", _("TEXT Files (*.txt)\t"),
                                              Fl_File_Chooser::SINGLE, warn.c_str());
    fc->textsize(9);
    fc->show();
    while (fc->shown())
        Fl::wait();  // block until choice is done
    if ((fc->value() != NULL)) {
#ifdef _DEBUG
        // printf("currentsound %i,roller %f, importon %i to %i :
        // %s\n",currentsound,Rollers[currentsound]->value(),((Fl_Input_Choice*)e)->menubutton()->value(),(int)memDisplay[currentsound]->value(),fc->value());//Speicher.multis[currentmulti].sound[currentsound]
        fflush(stdout);
#endif
        Fl::lock();
        fl_cursor(FL_CURSOR_WAIT, FL_WHITE, FL_BLACK);
        Fl::check();

        Speicher.importSound(fc->value(), (int)memDisplay[currentsound]->value());  // schoice[currentsound]->menubutton()->value());
        // ok, now we have a new sound saved but we should update the userinterface
        schoice[currentsound]->value(
            Speicher.getName(0, (int)memDisplay[currentsound]->value()).c_str());
        schoice[currentsound]->position(0);
        /*
        int i;
        for (i = 0;i<8;++i)
        {
            schoice[i]->clear();
        }

        for (i=0;i<512;++i)
        {
             if (i==230)
             {
                printf("%s\n",Speicher.getName(0,229).c_str());
                printf("%s\n",Speicher.getName(0,230).c_str());
                printf("%s\n",Speicher.getName(0,231).c_str());
             }
            schoice[0]->add(Speicher.getName(0,i).c_str());
            schoice[1]->add(Speicher.getName(0,i).c_str());
            schoice[2]->add(Speicher.getName(0,i).c_str());
            schoice[3]->add(Speicher.getName(0,i).c_str());
            schoice[4]->add(Speicher.getName(0,i).c_str());
            schoice[5]->add(Speicher.getName(0,i).c_str());
            schoice[6]->add(Speicher.getName(0,i).c_str());
            schoice[7]->add(Speicher.getName(0,i).c_str());
        }
        */
        fl_cursor(FL_CURSOR_DEFAULT, FL_WHITE, FL_BLACK);
        Fl::check();
        Fl::awake();
        Fl::unlock();
    }
}
/** callback when the storebutton is pressed
 * @param Fl_Widget the calling widget
 * @param defined by FLTK but not used
 */
static void storesound(Fl_Widget *o, void *e)
{
    Fl::lock();
    fl_cursor(FL_CURSOR_WAIT, FL_WHITE, FL_BLACK);
    Fl::check();
    int Speicherplatz = (int)memDisplay[currentsound]->value();
#ifdef _DEBUG
    printf("choice %i\n", Speicherplatz);  //((Fl_Input_Choice*)e)->menubutton()->value());
    fflush(stdout);
#endif
    Speicher.setChoice(currentsound, Speicherplatz);  //(Fl_Input_Choice*)e)->menubutton()->value());
    // clean first the name string
    sprintf(Speicher.sounds[Speicher.getChoice(currentsound)].name, "%s",
            ((Fl_Input *)e)->value());
#ifdef _DEBUG
    printf("input choice %s\n", ((Fl_Input *)e)->value());
#endif
    //((Fl_Input_Choice*)e)->menubutton()->replace(Speicher.getChoice(currentsound),((Fl_Input_Choice*)e)->value());

    // Schaltbrett.soundchoice-> add(Speicher.getName(i).c_str());
    int i;
    for (i = 0; i < _PARACOUNT; ++i)  // go through all parameters
    {
        if (Knob[currentsound][i] != NULL) {
            // int j=-1024;
            Speicher.sounds[Speicher.getChoice(currentsound)].parameter[i] =
                ((Fl_Valuator *)Knob[currentsound][i])->value();

            switch (i) {

            case 2:
            case 4:  // boost button
            case 15:
            case 17:
                // the repeat buttons of the mod egs
            case 64:
            case 69:
            case 74:
            case 79:
            case 84:
            case 89:
            case 115:

            {
                if (((Fl_Light_Button *)Knob[currentsound][i])->value() == 0) {
                    Speicher.sounds[Speicher.getChoice(currentsound)].parameter[i] = 0;
                }
                else {
                    Speicher.sounds[Speicher.getChoice(currentsound)].parameter[i] = 1;
                }
#ifdef _DEBUG
                printf("button %d = %f\n", i,
                       Speicher.sounds[Speicher.getChoice(currentsound)].parameter[i]);
#endif
                break;
            }
            case 3: {
                Speicher.sounds[Speicher.getChoice(currentsound)].freq[0][0] =
                    ((Fl_Positioner *)Knob[currentsound][i])->xvalue();
                Speicher.sounds[Speicher.getChoice(currentsound)].freq[0][1] =
                    ((Fl_Positioner *)Knob[currentsound][i])->yvalue();
                break;
            }
            case 18: {
                Speicher.sounds[Speicher.getChoice(currentsound)].freq[1][0] =
                    ((Fl_Positioner *)Knob[currentsound][i])->xvalue();
                Speicher.sounds[Speicher.getChoice(currentsound)].freq[1][1] =
                    ((Fl_Positioner *)Knob[currentsound][i])->yvalue();
                break;
            }
            //************************************ filter cuts *****************************
            case 30: {
                Speicher.sounds[Speicher.getChoice(currentsound)].freq[2][0] =
                    ((Fl_Positioner *)Knob[currentsound][i])->xvalue();
                Speicher.sounds[Speicher.getChoice(currentsound)].freq[2][1] =
                    ((Fl_Positioner *)Knob[currentsound][i])->yvalue();
                break;
            }
            case 33: {
                Speicher.sounds[Speicher.getChoice(currentsound)].freq[3][0] =
                    ((Fl_Positioner *)Knob[currentsound][i])->xvalue();
                Speicher.sounds[Speicher.getChoice(currentsound)].freq[3][1] =
                    ((Fl_Positioner *)Knob[currentsound][i])->yvalue();
                break;
            }
            case 40: {
                Speicher.sounds[Speicher.getChoice(currentsound)].freq[4][0] =
                    ((Fl_Positioner *)Knob[currentsound][i])->xvalue();
                Speicher.sounds[Speicher.getChoice(currentsound)].freq[4][1] =
                    ((Fl_Positioner *)Knob[currentsound][i])->yvalue();
                break;
            }
            case 43: {
                Speicher.sounds[Speicher.getChoice(currentsound)].freq[5][0] =
                    ((Fl_Positioner *)Knob[currentsound][i])->xvalue();
                Speicher.sounds[Speicher.getChoice(currentsound)].freq[5][1] =
                    ((Fl_Positioner *)Knob[currentsound][i])->yvalue();
                break;
            }
            case 50: {
                Speicher.sounds[Speicher.getChoice(currentsound)].freq[6][0] =
                    ((Fl_Positioner *)Knob[currentsound][i])->xvalue();
                Speicher.sounds[Speicher.getChoice(currentsound)].freq[6][1] =
                    ((Fl_Positioner *)Knob[currentsound][i])->yvalue();
                break;
            }
            case 53: {
                Speicher.sounds[Speicher.getChoice(currentsound)].freq[7][0] =
                    ((Fl_Positioner *)Knob[currentsound][i])->xvalue();
                Speicher.sounds[Speicher.getChoice(currentsound)].freq[7][1] =
                    ((Fl_Positioner *)Knob[currentsound][i])->yvalue();
                break;
            }
            case 90: {
                Speicher.sounds[Speicher.getChoice(currentsound)].freq[8][0] =
                    ((Fl_Positioner *)Knob[currentsound][i])->xvalue();
                Speicher.sounds[Speicher.getChoice(currentsound)].freq[8][1] =
                    ((Fl_Positioner *)Knob[currentsound][i])->yvalue();
                break;
            }
            // special treatment for the mix knobs, they are saved in the multisetting
            case 101:
            case 106:
            case 107:
            case 108:
            case 109: {
                // do nothing
            } break;

                //	{
                //		if (((Fl_Light_Button *)Knob[i])->value()==0)
                //		{
                //			Speicher.sounds[Speicher.getChoice()].parameter[i]=1;
                //		}
                //		else
                //		{
                //			Speicher.sounds[Speicher.getChoice()].parameter[i]=1000;
                //		}
                //	break;
                //	}
                /*
                case 60:
                //case 61:
                case 63:
                case 65:
                //case 66:
                case 68:

                case 70:
                //case 71:
                case 73:
                case 75:
                //case 76:
                case 78:

                case 80:
                //case 81:
                case 83:
                case 85:
                //case 86:
                case 88:


                case 102:
                case 105:
                {
                    float tr=(((Fl_Valuator*)o)->value());///200.f;//exp(((Fl_Valuator*)o)->value())/200.f;
                    tr *= tr*tr/2.f;// tr * tr*20.f;//48000.0f;//trtr*tr/2;
                    Speicher.sounds[Speicher.getChoice()].parameter[i]=tr;
                    break;
                }
                */

            default: {
                Speicher.sounds[Speicher.getChoice(currentsound)].parameter[i] =
                    ((Fl_Valuator *)Knob[currentsound][i])->value();
                break;
            }


            }  // end of switch
        }  // end of if
    }  // end of for

    for (i = 0; i < 17; ++i) {
        if (auswahl[currentsound][i] != NULL) {
            Speicher.sounds[Speicher.getChoice(currentsound)].choice[i] =
                auswahl[currentsound][i]->value();
#ifdef _DEBUG
            printf("f:%i:%i ", i, auswahl[currentsound][i]->value());
#endif
        }
    }
#ifdef _DEBUG
    printf("\n");
    fflush(stdout);
#endif
    Speicher.save();
    /*
    // ok, now we have saved but we should update the userinterface
    for (i = 0;i<8;++i)
    {
        schoice[i]->clear();
    }

    for (i=0;i<512;++i)
    {
        schoice[0]->add(Speicher.getName(0,i).c_str());
        schoice[1]->add(Speicher.getName(0,i).c_str());
        schoice[2]->add(Speicher.getName(0,i).c_str());
        schoice[3]->add(Speicher.getName(0,i).c_str());
        schoice[4]->add(Speicher.getName(0,i).c_str());
        schoice[5]->add(Speicher.getName(0,i).c_str());
        schoice[6]->add(Speicher.getName(0,i).c_str());
        schoice[7]->add(Speicher.getName(0,i).c_str());
    }
    */
    fl_cursor(FL_CURSOR_DEFAULT, FL_WHITE, FL_BLACK);
    Fl::check();
    Fl::awake();
    Fl::unlock();
}
/**
 * recall a single sound
 */
static void recall(unsigned int preset)
{
    int i;  //,j=-1024;
#ifdef _DEBUG
    printf("choice %i %i\n", currentsound, preset);  //((Fl_Input_Choice*)e)->menubutton()->value());
    fflush(stdout);
#endif
    Speicher.setChoice(currentsound, preset);
    for (i = 0; i < _PARACOUNT; ++i) {
        if (Knob[currentsound][i] != NULL) {
#ifdef _DEBUG
            printf("i == %i \n", i);
            fflush(stdout);
#endif
            switch (i) {


            case 2:
            case 4:  // boost button
            case 15:
            case 17:
                // the repeat buttons of the mod egs
            case 64:
            case 69:
            case 74:
            case 79:
            case 84:
            case 89:
            case 115:

            {
#ifdef _DEBUG
                printf("handle: %d\n", i);
#endif
                if (Speicher.sounds[Speicher.getChoice(currentsound)].parameter[i] == 0.0f) {
                    ((Fl_Light_Button *)Knob[currentsound][i])->value(0);
                }
                else {
                    ((Fl_Light_Button *)Knob[currentsound][i])->value(1);
                }
                callback(Knob[currentsound][i], NULL);

                break;
            }
                //{
                //	((Fl_Light_Button*)Knob[i])->value((int)Speicher.sounds[Speicher.getChoice()].parameter[i]);

                //	break;
                //}

            case 3: {
                ((Fl_Positioner *)Knob[currentsound][i])
                    ->xvalue(Speicher.sounds[Speicher.getChoice(currentsound)].freq[0][0]);
                ((Fl_Positioner *)Knob[currentsound][i])
                    ->yvalue(Speicher.sounds[Speicher.getChoice(currentsound)].freq[0][1]);
                callback(Knob[currentsound][i], NULL);

                break;
            }
            case 18: {
                ((Fl_Positioner *)Knob[currentsound][i])
                    ->xvalue(Speicher.sounds[Speicher.getChoice(currentsound)].freq[1][0]);
                ((Fl_Positioner *)Knob[currentsound][i])
                    ->yvalue(Speicher.sounds[Speicher.getChoice(currentsound)].freq[1][1]);
                callback(Knob[currentsound][i], NULL);
                break;
            }
            //************************************ filter cuts *****************************
            case 30: {
                ((Fl_Positioner *)Knob[currentsound][i])
                    ->xvalue(Speicher.sounds[Speicher.getChoice(currentsound)].freq[2][0]);
                ((Fl_Positioner *)Knob[currentsound][i])
                    ->yvalue(Speicher.sounds[Speicher.getChoice(currentsound)].freq[2][1]);
                callback(Knob[currentsound][i], NULL);

                break;
            }
            case 33: {
                ((Fl_Positioner *)Knob[currentsound][i])
                    ->xvalue(Speicher.sounds[Speicher.getChoice(currentsound)].freq[3][0]);
                ((Fl_Positioner *)Knob[currentsound][i])
                    ->yvalue(Speicher.sounds[Speicher.getChoice(currentsound)].freq[3][1]);
                callback(Knob[currentsound][i], NULL);

                break;
            }
            case 40: {
                ((Fl_Positioner *)Knob[currentsound][i])
                    ->xvalue(Speicher.sounds[Speicher.getChoice(currentsound)].freq[4][0]);
                ((Fl_Positioner *)Knob[currentsound][i])
                    ->yvalue(Speicher.sounds[Speicher.getChoice(currentsound)].freq[4][1]);
                callback(Knob[currentsound][i], NULL);
                break;
            }
            case 43: {
                ((Fl_Positioner *)Knob[currentsound][i])
                    ->xvalue(Speicher.sounds[Speicher.getChoice(currentsound)].freq[5][0]);
                ((Fl_Positioner *)Knob[currentsound][i])
                    ->yvalue(Speicher.sounds[Speicher.getChoice(currentsound)].freq[5][1]);
                callback(Knob[currentsound][i], NULL);

                break;
            }
            case 50: {
                ((Fl_Positioner *)Knob[currentsound][i])
                    ->xvalue(Speicher.sounds[Speicher.getChoice(currentsound)].freq[6][0]);
                ((Fl_Positioner *)Knob[currentsound][i])
                    ->yvalue(Speicher.sounds[Speicher.getChoice(currentsound)].freq[6][1]);
                callback(Knob[currentsound][i], NULL);

                break;
            }
            case 53: {
                ((Fl_Positioner *)Knob[currentsound][i])
                    ->xvalue(Speicher.sounds[Speicher.getChoice(currentsound)].freq[7][0]);
                ((Fl_Positioner *)Knob[currentsound][i])
                    ->yvalue(Speicher.sounds[Speicher.getChoice(currentsound)].freq[7][1]);
                callback(Knob[currentsound][i], NULL);

                break;
            }
            case 90: {
                ((Fl_Positioner *)Knob[currentsound][i])
                    ->xvalue(Speicher.sounds[Speicher.getChoice(currentsound)].freq[8][0]);
                ((Fl_Positioner *)Knob[currentsound][i])
                    ->yvalue(Speicher.sounds[Speicher.getChoice(currentsound)].freq[8][1]);
                callback(Knob[currentsound][i], NULL);
                break;
            }

            // special treatment for the mix knobs, they are saved in the multisetting
            case 101:
            case 106:
            case 107:
            case 108:
            case 109: {
                // do nothing
            } break;
            default: {
                ((Fl_Valuator *)Knob[currentsound][i])
                    ->value(Speicher.sounds[Speicher.getChoice(currentsound)].parameter[i]);
                callback(Knob[currentsound][i], NULL);
                break;
            }
            }
        }
    }

#ifdef _DEBUG
    printf("so weit so gut");
#endif
    for (i = 0; i < 17; ++i) {
        if (auswahl[currentsound][i] != NULL) {
            auswahl[currentsound][i]->value(
                Speicher.sounds[Speicher.getChoice(currentsound)].choice[i]);
            choicecallback(auswahl[currentsound][i], NULL);
#ifdef _DEBUG
            printf("l:%i:%i ", i,
                   Speicher.sounds[Speicher.getChoice(currentsound)].choice[i]);
#endif
        }
    }
    // send a reset
    sendParameter(currentsound, 0, 0.0f);
#ifdef _DEBUG
    printf("\n");
    fflush(stdout);
#endif
}
/**
 * callback when the load button is pressed
 * @param pointer to the calling widget
 * @param optional data, this time the entry id of which the sound
 * should be loaded
 */
static void loadsound(Fl_Widget *o, void *)
{
    Fl::lock();
    // fl_cursor(FL_CURSOR_WAIT,FL_WHITE, FL_BLACK);
    // Fl::awake();
#ifdef _DEBUG
    // printf("maybe %i choice %i\n",Speicher.getChoice(currentsound),((Fl_Input_Choice*)e)->menubutton()->value());
    fflush(stdout);
#endif
    // Speicher.multis[currentmulti].sound[currentsound]=(unsigned int)((Fl_Input_Choice*)e)->menubutton()->value();
    // recall(Speicher.multis[currentmulti].sound[currentsound]);
    recall((unsigned int)((int)memDisplay[currentsound]->value()));  //(Fl_Input_Choice*)e)->menubutton()->value());
    // fl_cursor(FL_CURSOR_DEFAULT,FL_WHITE, FL_BLACK);
    Fl::awake();
    Fl::unlock();
}
/**
 * callback when the load multi button is pressed
 * recall a multitemperal setup
 * @param pointer to the calling widget
 * @param optional data, this time the entry id of which the sound
 * should be loaded
 */
static void loadmulti(Fl_Widget *o, void *e)
{
    Fl::lock();
    // fl_cursor(FL_CURSOR_WAIT ,FL_WHITE, FL_BLACK);
    // Fl::awake();
    currentmulti = (unsigned int)multiRoller->value();
    // multi[currentmulti][currentsound]=(unsigned int)((Fl_Input_Choice*)e)->menubutton()->value();
    for (int i = 0; i < 8; ++i) {

        currentsound = i;
        if ((Speicher.multis[currentmulti].sound[i] >= 0) &&
            (Speicher.multis[currentmulti].sound[i] < 512)) {
            recall(Speicher.multis[currentmulti].sound[i]);  // actual recall Bug
#ifdef _DEBUG
            printf("i ist %i Speicher ist %i\n", i,
                   Speicher.multis[currentmulti].sound[i]);
            fflush(stdout);
#endif
            schoice[i]->value(
                Speicher.getName(0, Speicher.multis[currentmulti].sound[i]).c_str());  // set gui
            schoice[i]->position(0);  // set back the cursor to pos 0 to show name from the beginning
#ifdef _DEBUG
            printf("schoice gesetzt\n");
            fflush(stdout);
#endif
            Rollers[i]->value(Speicher.multis[currentmulti].sound[i]);  // set gui
#ifdef _DEBUG
            printf("Roller gesetzt\n");
            fflush(stdout);
#endif
            memDisplay[i]->value(Speicher.multis[currentmulti].sound[i]);
#ifdef _DEBUG
            printf("memDisplay gesetzt\n");
            fflush(stdout);
#endif
        }
        // set the knobs of the mix
        for (int j = 0; j < _MULTISETTINGS; ++j) {
            int actualknob;
            // a little unelegant translating of the array pos to the actual knob number, got to rethink this part
            switch (j) {
            case 0:  // knob 101
                ((Fl_Valuator *)Knob[i][101])
                    ->value(Speicher.multis[currentmulti].settings[i][j]);
                callback(Knob[i][101], NULL);
                break;
            case 1:  // knob 106
            case 2:  // knob 107
            case 3:  // knob 108
            case 4:  // knob 109
            {
                actualknob = j + 105;
#ifdef _DEBUG
                printf("i:%i j:%i knob:%i\n", i, j, actualknob);
                fflush(stdout);
#endif
                ((Fl_Valuator *)Knob[i][actualknob])
                    ->value(Speicher.multis[currentmulti].settings[i][j]);
                callback(Knob[i][actualknob], NULL);
            } break;
            default:
                // do nothing
                break;
            }
        }
    }
    currentsound = 0;
    // we should go to a defined state, means tab
    tabs->value(tab[0]);
    tabcallback(tabs, NULL);

#ifdef _DEBUG
    printf("multi choice %s\n", ((Fl_Input *)e)->value());
    fflush(stdout);
#endif
    // fl_cursor(FL_CURSOR_DEFAULT,FL_WHITE, FL_BLACK);
    // Fl::check();
    Fl::awake();
    Fl::unlock();
}

/**
 * callback when the store multi button is pressed
 * store a multitemperal setup
 * @param pointer to the calling widget
 */
static void storemulti(Fl_Widget *o, void *e)
{
    Fl::lock();
    fl_cursor(FL_CURSOR_WAIT, FL_WHITE, FL_BLACK);
    Fl::check();
    /*printf("choice %i\n",((Fl_Input_Choice*)e)->menubutton()->value());
    fflush(stdout);
    */
    int i;

    if (Multichoice != NULL) {
        int t = (int)multiRoller->value();  // Multichoice->menubutton()->value();
#ifdef _DEBUG
        printf("was:%d is:%d\n", currentmulti, t);
#endif
        if ((t != currentmulti) && (t > -1) && (t < 128)) {
            currentmulti = t;
        }
    }
    else
        printf("problems with the multichoice widgets!\n");

    strcpy(Speicher.multis[currentmulti].name, ((Fl_Input *)e)->value());
    // printf("input choice %s\n",((Fl_Input_Choice*)e)->value());

    //((Fl_Input_Choice*)e)->menubutton()->replace(currentmulti,((Fl_Input_Choice*)e)->value());

    // Schaltbrett.soundchoice-> add(Speicher.getName(i).c_str());
    // get the knobs of the mix

    for (i = 0; i < 8; ++i) {
        Speicher.multis[currentmulti].sound[i] = Speicher.getChoice(i);
#ifdef _DEBUG
        printf("sound slot: %d = %d\n", i, Speicher.getChoice(i));
#endif
        Speicher.multis[currentmulti].settings[i][0] =
            ((Fl_Valuator *)Knob[i][101])->value();
        Speicher.multis[currentmulti].settings[i][1] =
            ((Fl_Valuator *)Knob[i][106])->value();
        Speicher.multis[currentmulti].settings[i][2] =
            ((Fl_Valuator *)Knob[i][107])->value();
        Speicher.multis[currentmulti].settings[i][3] =
            ((Fl_Valuator *)Knob[i][108])->value();
        Speicher.multis[currentmulti].settings[i][4] =
            ((Fl_Valuator *)Knob[i][109])->value();
    }
    // write to disk
    Speicher.saveMulti();

    fl_cursor(FL_CURSOR_DEFAULT, FL_WHITE, FL_BLACK);
    Fl::check();
    Fl::awake();
    Fl::unlock();
}
/*
static void voicecallback(Fl_Widget* o, void* e)
{
    transmit=false;
    currentsound=((unsigned int)((Fl_Valuator*)o)->value());
    recall(multi[currentmulti][currentsound]);
    transmit=true;
}*/
/**
 * change the multisetup,should be called from a Midi Program Change event on Channel 9
 * @param int the Program number between 0 and 127
 */
void UserInterface::changeMulti(int pgm)
{
    Fl::lock();
    multichoice->value(Speicher.multis[pgm].name);
    // multichoice->damage(FL_DAMAGE_ALL);
    // multichoice->redraw();
    multiRoller->value(pgm);  // set gui
    multiRoller->redraw();
    multiDisplay->value(pgm);
    loadmulti(NULL, multichoice);
    //	Fl::redraw();
    //	Fl::flush();
    Fl::awake();
    Fl::unlock();
}
/**
 * change the sound for a certain voice,should be called from a Midi Program Change event on Channel 1 - 8
 * @param int the voice between 0 and 7 (it is not clear if the first Midi channel is 1 (which is usually the case in the hardware world) or 0)
 * @param int the Program number between 0 and 127
 */
void UserInterface::changeSound(int channel, int pgm)
{
    if ((channel > -1) && (channel < 8) && (pgm > -1) && (pgm < 128)) {
        Fl::lock();
        int t = currentsound;
        currentsound = channel;
        schoice[channel]->value(Speicher.getName(0, pgm).c_str());
        schoice[channel]->position(0);
        // schoice[channel]->damage(FL_DAMAGE_ALL);
        // schoice[channel]->redraw();
        Rollers[channel]->value(pgm);  // set gui
        Rollers[channel]->redraw();
        memDisplay[channel]->value(pgm);
        loadsound(NULL, schoice[channel]);
        //		Fl::redraw();
        //		Fl::flush();
        currentsound = t;
        Fl::awake();
        Fl::unlock();
    }
}

/*
Fl_SteinerKnob::Fl_SteinerKnob(int x, int y, int w, int h, const char *label)
: Fl_Dial(x, y, w, h, label) {

}
int Fl_SteinerKnob::handle(int event) {
    if (event==FL_PUSH)
    {
        if (Fl::event_button2())
        {
            altx=Fl::event_x();

            this->set_changed();
            //return 0;
        }
        else if (Fl::event_button3())
        {
            this->step(0.001);
        }
        else this->step(0);
    }
    if (event ==FL_DRAG)
    {
    if (Fl::event_button2())
        {
            if (Fl::event_x()<altx )
             this->value(this->value()+1);
            else   this->value(this->value()-1);
            altx=Fl::event_x();
            this->set_changed();
            return 0;
        }
        if (Fl::event_button3())
        {
            if (Fl::event_x()<altx )
             this->value(this->value()+0.01);
            else   this->value(this->value()-0.01);
            altx=Fl::event_x();
            this->set_changed();
            return 0;
        }
    }
      return Fl_Dial::handle(event);
}
*/
// ---------------------------------------------------------------
// 			screen initialization
// ---------------------------------------------------------------

Fenster *UserInterface::make_window()
{
    // Fl_Double_Window* w;
    // {
    currentsound = 0;
    currentmulti = 0;
    enableTransmit(true);
    Fenster *o = new Fenster(995, 515);
    o->label(_("Minicomputer"));
    // w = o;
    o->color((Fl_Color)246);
    o->user_data((void *)(this));
    for (int i = 0; i < 17; ++i) {
#ifdef _DEBUG
        printf("%i \n", i);
#endif
        auswahl[currentsound][i] = NULL;
        fflush(stdout);
    }
    for (int i = 0; i < _PARACOUNT; ++i) {
        Knob[currentsound][i] = NULL;
    }

    // menus ---------------------------------------------------------------------
    /**
     * predefined menue with all modulation sources
     */
    static Fl_Menu_Item menu_amod[] = {
        {_("none"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("velocity"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("pitch bend"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("osc 1 fm out"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("osc 2 fm out"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("osc 1"), 0, 0, 0, FL_MENU_INVISIBLE, FL_NORMAL_LABEL, 0, 8, 0},
        {_("osc 2"), 0, 0, 0, FL_MENU_INVISIBLE, FL_NORMAL_LABEL, 0, 8, 0},
        {_("filter"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("eg 1"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("eg 2"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("eg 3"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("eg 4"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("eg 5"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("eg 6"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("modulation osc"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("touch"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("mod wheel"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("cc 12"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("delay"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("midi note"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("cc 2"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("cc 4"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("cc 5"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("cc 6"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("cc 14"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("cc 15"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("cc 16"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("cc 17"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0}};

    // redundant for now...
    static Fl_Menu_Item menu_fmod[] = {
        {_("none"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("velocity"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("pitch bend"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("osc 1 fm out"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("osc 2 fm out"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("osc 1"), 0, 0, 0, FL_MENU_INVISIBLE, FL_NORMAL_LABEL, 0, 8, 0},
        {_("osc 2"), 0, 0, 0, FL_MENU_INVISIBLE, FL_NORMAL_LABEL, 0, 8, 0},
        {_("filter"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("eg 1"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("eg 2"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("eg 3"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("eg 4"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("eg 5"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("eg 6"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("modulation osc"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("touch"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("mod wheel"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("cc 12"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("delay"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("midi note"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("cc 2"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("cc 4"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("cc 5"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("cc 6"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("cc 14"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("cc 15"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("cc 16"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("cc 17"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0}};

    /**
     * waveform list for menue
     */
    static Fl_Menu_Item menu_wave[] = {
        {_("sine"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("ramp up"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("ramp down"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("tri"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("square"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("bit"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("spike"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("comb"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("add Saw"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("add Square"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("Microcomp 1"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("Microcomp 8"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("Microcomp 9"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("Microcomp 10"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("Microcomp 11"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("Microcomp 12"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("Microcomp 13"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {_("Microcomp 14"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 8, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0}};

#if 0
    static Fl_Menu_Item menu_pitch[] = {{_("EG1"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
                                        {_("velocity"), 0, 0, 0, 0,
                                         FL_NORMAL_LABEL, 0, 14, 0},
                                        {0, 0, 0, 0, 0, 0, 0, 0, 0}};


    static Fl_Menu_Item menu_pitch1[] = {{_("EG1"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
                                         {_("velocity"), 0, 0, 0, 0,
                                          FL_NORMAL_LABEL, 0, 14, 0},
                                         {0, 0, 0, 0, 0, 0, 0, 0, 0}};

    static Fl_Menu_Item menu_morph[] = {{_("EG1"), 0, 0, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
                                        {_("velocity"), 0, 0, 0, 0,
                                         FL_NORMAL_LABEL, 0, 14, 0},
                                        {0, 0, 0, 0, 0, 0, 0, 0, 0}};
#endif

    // tabs beginning ------------------------------------------------------------
    {
        Fl_Tabs *o = new Fl_Tabs(0, 0, 995, 515);
        o->callback((Fl_Callback *)tabcallback);
        int i;
        for (i = 0; i < 8; ++i)  // generate 8 tabs for the 8 voices
        {
            {
                ostringstream oss;
                oss << _("sound ") << (i + 1);  // create name for tab
                tablabel[i] = oss.str();
                Fl_Group *o = new Fl_Group(1, 10, 995, 515, tablabel[i].c_str());
                o->color((Fl_Color)246);
                o->labelsize(8);
                // o->argument(2);
                // int xtab=1;
                // o->labelcolor(FL_BACKGROUND2_COLOR);
                // o->callback((Fl_Callback*)tabcallback,&xtab);

                o->box(FL_BORDER_FRAME);
                // draw logo
                {
                    Fl_Box *o = new Fl_Box(855, 450, 25, 25);
                    o->image(image_miniMini);
                }
                {
                    Fl_Group *o = new Fl_Group(5, 17, 300, 212);
                    o->box(FL_ROUNDED_FRAME);
                    o->color(FL_BACKGROUND2_COLOR);
                    {
                        Fl_Box *d = new Fl_Box(145, 210, 30, 22, _("oscillator 1"));
                        d->labelsize(8);
                        d->labelcolor(FL_BACKGROUND2_COLOR);
                    }
                    {
                        Mw_Dial *o = new Mw_Dial(21, 20, 34, 34, _("frequency"));
                        o->labelsize(8);
                        o->maximum(1000);
                        o->argument(1);
                        o->callback((Fl_Callback *)callback);
                        Knob[i][1] = o;
                    }
                    {
                        Fl_Value_Input *o = new Fl_Value_Input(16, 66, 46, 15);  // frequency display of oscillator 1
                        o->box(FL_ROUNDED_BOX);
                        o->labelsize(8);
                        o->textsize(8);
                        o->maximum(10000);
                        o->step(0.001);
                        o->argument(1);
                        o->callback((Fl_Callback *)callback);
                        miniDisplay[i][0] = o;
                    }
                    {
                        Fl_Light_Button *o =
                            new Fl_Light_Button(20, 92, 72, 19, _("fix frequency"));
                        o->box(FL_BORDER_BOX);
                        o->selection_color((Fl_Color)89);
                        o->labelsize(8);
                        o->argument(2);
                        o->callback((Fl_Callback *)callback);
                        Knob[i][2] = o;
                    }
                    {
                        Mw_Dial *o = new Mw_Dial(79, 179, 25, 25, _("fm output  vol"));
                        o->labelsize(8);
                        o->argument(13);
                        // o->maximum(1000);
                        o->callback((Fl_Callback *)callback);
                        Knob[i][o->argument()] = o;
                    }
                    {
                        Fl_Light_Button *o = new Fl_Light_Button(80, 27, 40, 15, _("boost"));
                        o->box(FL_BORDER_BOX);
                        o->selection_color((Fl_Color)89);
                        o->labelsize(8);
                        o->argument(4);
                        o->callback((Fl_Callback *)callback);
                        Knob[i][o->argument()] = o;
                    }
                    /*{ Fl_Dial* o = new Fl_SteinerKnob(20, 121, 34, 34,
                    _("tune")); o->labelsize(8); o->minimum(0.5); o->maximum(16);
                      o->argument(3);
                      o->callback((Fl_Callback*)callback);
                      Knob[3] = o;
                    }*/

                    {
                        Fl_Positioner *o = new Fl_Positioner(15, 121, 40, 80, _("tune"));
                        o->xbounds(0, 16);
                        o->ybounds(1, 0);
                        o->box(FL_BORDER_BOX);
                        o->xstep(1);
                        o->labelsize(8);
                        o->argument(3);
                        o->selection_color(0);
                        o->callback((Fl_Callback *)callback);
                        Knob[i][3] = o;
                    }
                    {
                        Fl_Value_Input *o = new Fl_Value_Input(62, 130, 46, 15);
                        o->box(FL_ROUNDED_BOX);
                        o->labelsize(8);
                        o->textsize(8);
                        o->maximum(10000);
                        o->argument(3);
                        o->step(0.001);
                        o->callback((Fl_Callback *)tuneCallback);
                        miniDisplay[i][1] = o;
                    }
                    {
                        Mw_Dial *o = new Mw_Dial(260, 97, 25, 25, _("amount"));
                        o->labelsize(8);
                        o->argument(9);
                        o->minimum(-1);
                        o->maximum(1);
                        o->callback((Fl_Callback *)callback);
                        Knob[i][o->argument()] = o;
                    }
                    {
                        Fl_Choice *o = new Fl_Choice(134, 102, 120, 15, _("amp modulator 1"));
                        o->box(FL_BORDER_BOX);
                        o->down_box(FL_BORDER_BOX);
                        o->labelsize(8);
                        o->textsize(8);
                        o->align(FL_ALIGN_TOP_LEFT);
                        o->argument(2);
                        o->callback((Fl_Callback *)choicecallback);
                        o->menu(menu_amod);
                        auswahl[i][o->argument()] = o;
                    }
                    {
                        Mw_Dial *o = new Mw_Dial(260, 133, 25, 25, _("amount"));
                        o->labelsize(8);
                        o->argument(11);
                        o->minimum(-1);
                        o->maximum(1);
                        o->callback((Fl_Callback *)callback);
                        Knob[i][o->argument()] = o;
                    }
                    {
                        Fl_Choice *o = new Fl_Choice(134, 138, 120, 15, _("amp modulator 2"));
                        o->box(FL_BORDER_BOX);
                        o->down_box(FL_BORDER_BOX);
                        o->labelsize(8);
                        o->textsize(8);
                        o->align(FL_ALIGN_TOP_LEFT);
                        o->argument(3);
                        o->callback((Fl_Callback *)choicecallback);
                        o->menu(menu_amod);
                        auswahl[i][o->argument()] = o;
                    }
                    {
                        Mw_Dial *o = new Mw_Dial(247, 23, 25, 25, _("amount"));
                        o->labelsize(8);
                        o->argument(5);
                        o->minimum(-1000);
                        o->maximum(1000);
                        o->callback((Fl_Callback *)callback);
                        Knob[i][o->argument()] = o;
                    }
                    {
                        Fl_Choice *o = new Fl_Choice(122, 28, 120, 15, _("freq modulator 1"));
                        o->box(FL_BORDER_BOX);
                        o->down_box(FL_BORDER_BOX);
                        o->labelsize(8);
                        o->textsize(8);
                        o->align(FL_ALIGN_TOP_LEFT);
                        o->argument(0);
                        o->callback((Fl_Callback *)choicecallback);
                        o->menu(menu_fmod);
                        auswahl[i][o->argument()] = o;
                    }
                    {
                        Mw_Dial *o = new Mw_Dial(248, 59, 25, 25, _("amount"));
                        o->labelsize(8);
                        o->argument(7);
                        o->minimum(-1000);
                        o->maximum(1000);
                        o->callback((Fl_Callback *)callback);
                        Knob[i][o->argument()] = o;
                    }
                    {
                        Fl_Choice *o = new Fl_Choice(122, 64, 120, 15, _("freq modulator 2"));
                        o->box(FL_BORDER_BOX);
                        o->down_box(FL_BORDER_BOX);
                        o->labelsize(8);
                        o->textsize(8);
                        o->align(FL_ALIGN_TOP_LEFT);
                        o->argument(1);
                        o->callback((Fl_Callback *)choicecallback);
                        o->menu(menu_fmod);
                        auswahl[i][o->argument()] = o;
                    }


                    {
                        Fl_Choice *j = new Fl_Choice(120, 184, 120, 15, _("waveform"));
                        j->box(FL_BORDER_BOX);
                        j->down_box(FL_BORDER_BOX);
                        j->labelsize(8);
                        j->textsize(8);
                        j->align(FL_ALIGN_TOP_LEFT);
                        j->argument(4);
                        auswahl[i][j->argument()] = j;
                        j->callback((Fl_Callback *)choicecallback);
                        j->menu(menu_wave);
                    }
                    o->end();
                }

                //}
                {
                    Fl_Group *o = new Fl_Group(5, 238, 300, 212);
                    o->box(FL_ROUNDED_FRAME);
                    o->color(FL_BACKGROUND2_COLOR);
                    {
                        Fl_Box *d = new Fl_Box(145, 431, 30, 22, _("oscillator 2"));
                        d->labelsize(8);
                        d->labelcolor(FL_BACKGROUND2_COLOR);
                    }
                    {
                        Mw_Dial *o = new Mw_Dial(21, 244, 34, 34, _("frequency"));
                        o->labelsize(8);
                        o->argument(16);
                        o->maximum(1000);
                        o->callback((Fl_Callback *)callback);
                        Knob[i][16] = o;
                    }
                    {
                        Fl_Value_Input *o = new Fl_Value_Input(16, 290, 46, 15);
                        o->box(FL_ROUNDED_BOX);
                        o->labelsize(8);
                        o->textsize(8);
                        o->maximum(10000);
                        o->step(0.001);
                        o->argument(16);
                        o->callback((Fl_Callback *)callback);
                        miniDisplay[i][2] = o;
                    }
                    {
                        Fl_Light_Button *o =
                            new Fl_Light_Button(20, 316, 72, 19, _("fix frequency"));
                        o->box(FL_BORDER_BOX);
                        o->selection_color((Fl_Color)89);
                        o->labelsize(8);
                        o->argument(17);
                        o->callback((Fl_Callback *)callback);
                        Knob[i][o->argument()] = o;
                    }
                    {
                        Mw_Dial *o = new Mw_Dial(79, 403, 25, 25, _("fm output  vol"));
                        o->labelsize(8);
                        o->argument(28);
                        //  o->maximum(1000);
                        o->callback((Fl_Callback *)callback);
                        Knob[i][28] = o;
                    }
                    {
                        Fl_Light_Button *o = new Fl_Light_Button(80, 251, 40, 15, _("boost"));
                        o->box(FL_BORDER_BOX);
                        o->selection_color((Fl_Color)89);
                        o->labelsize(8);
                        o->argument(15);
                        o->callback((Fl_Callback *)callback);
                        Knob[i][o->argument()] = o;
                    }
                    {
                        Fl_Positioner *o = new Fl_Positioner(15, 345, 40, 80, _("tune"));
                        o->xbounds(0, 16);
                        o->ybounds(1, 0);
                        o->box(FL_BORDER_BOX);
                        o->xstep(1);
                        o->selection_color(0);
                        o->labelsize(8);
                        o->argument(18);
                        o->callback((Fl_Callback *)callback);

                        /*Fl_Dial* o = new Fl_SteinerKnob(20, 345, 34, 34,
                        _("tune")); o->labelsize(8); o->minimum(0.5);
                        o->maximum(16);
                        o->argument(18);
                        o->callback((Fl_Callback*)callback);*/
                        Knob[i][18] = o;
                    }
                    {
                        Fl_Value_Input *o = new Fl_Value_Input(62, 354, 46, 15);
                        o->box(FL_ROUNDED_BOX);
                        o->labelsize(8);
                        o->textsize(8);
                        o->maximum(10000);
                        o->argument(18);
                        o->step(0.001);
                        o->callback((Fl_Callback *)tuneCallback);
                        miniDisplay[i][3] = o;
                    }
                    {
                        Mw_Dial *o = new Mw_Dial(260, 321, 25, 25, _("amount"));
                        o->labelsize(8);
                        o->argument(23);
                        o->minimum(-1);
                        o->maximum(1);
                        o->callback((Fl_Callback *)callback);
                        Knob[i][o->argument()] = o;
                    }
                    {
                        Fl_Choice *o = new Fl_Choice(134, 326, 120, 15, _("amp modulator"));
                        o->box(FL_BORDER_BOX);
                        o->down_box(FL_BORDER_BOX);
                        o->labelsize(8);
                        o->textsize(8);
                        o->align(FL_ALIGN_TOP_LEFT);
                        o->argument(8);
                        o->callback((Fl_Callback *)choicecallback);
                        o->menu(menu_amod);
                        auswahl[i][o->argument()] = o;
                    }
                    {
                        Mw_Dial *o = new Mw_Dial(260, 357, 25, 25, _("amount"));
                        o->labelsize(8);
                        o->argument(25);
                        o->minimum(-1);
                        o->maximum(1);
                        o->callback((Fl_Callback *)callback);
                        Knob[i][o->argument()] = o;
                    }
                    {
                        Fl_Choice *o = new Fl_Choice(134, 362, 120, 15, _("fm out amp modulator"));
                        o->box(FL_BORDER_BOX);
                        o->down_box(FL_BORDER_BOX);
                        o->labelsize(8);
                        o->textsize(8);
                        o->align(FL_ALIGN_TOP_LEFT);
                        o->argument(9);
                        o->callback((Fl_Callback *)choicecallback);
                        o->menu(menu_amod);
                        auswahl[i][o->argument()] = o;
                    }
                    {
                        Mw_Dial *o = new Mw_Dial(247, 247, 25, 25, _("amount"));
                        o->labelsize(8);
                        o->argument(19);
                        o->minimum(-1000);
                        o->maximum(1000);
                        o->callback((Fl_Callback *)callback);
                        Knob[i][o->argument()] = o;
                    }
                    {
                        Fl_Choice *o = new Fl_Choice(122, 252, 120, 15, _("freq modulator 1"));
                        o->box(FL_BORDER_BOX);
                        o->down_box(FL_BORDER_BOX);
                        o->labelsize(8);
                        o->textsize(8);
                        o->align(FL_ALIGN_TOP_LEFT);
                        o->argument(6);
                        o->callback((Fl_Callback *)choicecallback);
                        o->menu(menu_fmod);
                        auswahl[i][o->argument()] = o;
                    }
                    {
                        Mw_Dial *o = new Mw_Dial(248, 283, 25, 25, _("amount"));
                        o->labelsize(8);
                        o->argument(21);
                        o->minimum(-1000);
                        o->maximum(1000);
                        o->callback((Fl_Callback *)callback);
                        Knob[i][o->argument()] = o;
                    }
                    {
                        Fl_Choice *o = new Fl_Choice(122, 288, 120, 15, _("freq modulator 2"));
                        o->box(FL_BORDER_BOX);
                        o->down_box(FL_BORDER_BOX);
                        o->labelsize(8);
                        o->textsize(8);
                        o->align(FL_ALIGN_TOP_LEFT);
                        o->argument(7);
                        o->callback((Fl_Callback *)choicecallback);
                        o->menu(menu_fmod);
                        auswahl[i][o->argument()] = o;
                    }
                    {
                        Fl_Choice *o = new Fl_Choice(120, 408, 120, 15, _("waveform"));
                        o->box(FL_BORDER_BOX);
                        o->down_box(FL_BORDER_BOX);
                        o->labelsize(8);
                        o->textsize(8);
                        o->align(FL_ALIGN_TOP_LEFT);
                        o->argument(5);
                        o->callback((Fl_Callback *)choicecallback);
                        o->menu(menu_wave);
                        auswahl[i][o->argument()] = o;
                    }
                    {
                        Fl_Light_Button *o =
                            new Fl_Light_Button(220, 430, 65, 15, _("sync to osc1"));
                        o->box(FL_BORDER_BOX);
                        o->selection_color((Fl_Color)89);
                        o->labelsize(8);
                        o->argument(115);
                        o->callback((Fl_Callback *)callback);
                        Knob[i][o->argument()] = o;
                    }
                    o->end();
                }
                // ----------------- knobs  for the filters --------------------------------------
                {
                    Fl_Group *o = new Fl_Group(312, 17, 277, 433);
                    o->box(FL_ROUNDED_FRAME);
                    o->color(FL_BACKGROUND2_COLOR);
                    {
                        Fl_Box *d = new Fl_Box(312, 225, 277, 435, _("filters"));
                        d->labelsize(8);
                        d->labelcolor(FL_BACKGROUND2_COLOR);
                    }
                    {
                        Fl_Group *o = new Fl_Group(330, 28, 239, 92, _("filter 1"));
                        o->box(FL_ROUNDED_FRAME);
                        o->color(FL_FOREGROUND_COLOR);
                        o->labelsize(8);
                        {
                            Fl_Positioner *o = new Fl_Positioner(340, 31, 70, 79, _("cut"));
                            o->xbounds(0, 9000);
                            o->ybounds(499, 0);
                            o->selection_color(0);
                            o->box(FL_BORDER_BOX);
                            o->xstep(500);
                            o->labelsize(8);
                            o->argument(30);
                            o->yvalue(200.5);
                            o->callback((Fl_Callback *)callback);
                            Knob[i][o->argument()] = o;

                            /* Fl_Dial* o = f1cut1 = new Fl_SteinerKnob(344, 51,
                            34, 34, _("cut")); o->labelsize(8); o->argument(30);
                                o->maximum(10000);
                            o->value(50);
                            o->callback((Fl_Callback*)callback);
                            */
                        }
                        {
                            Mw_Dial *o = f1q1 = new Mw_Dial(415, 33, 25, 25, _("q"));
                            o->labelsize(8);
                            o->argument(31);
                            o->minimum(0.9);
                            o->value(0.9);
                            o->maximum(0.01);
                            o->callback((Fl_Callback *)callback);
                            Knob[i][o->argument()] = o;
                        }
                        {
                            Mw_Dial *o = f1vol1 = new Mw_Dial(425, 70, 20, 20, _("vol"));
                            o->labelsize(8);
                            o->argument(32);
                            o->callback((Fl_Callback *)callback);
                            o->minimum(-1);
                            o->value(0.5);
                            o->maximum(1);
                            Knob[i][o->argument()] = o;
                        }


                        {
                            Fl_Positioner *o = new Fl_Positioner(456, 31, 70, 79, _("cut"));
                            o->xbounds(0, 9000);
                            o->ybounds(499, 0);
                            o->box(FL_BORDER_BOX);
                            o->xstep(500);
                            o->selection_color(0);
                            o->labelsize(8);
                            o->yvalue(20);
                            o->callback((Fl_Callback *)callback);
                            /* Fl_Dial* o = f1cut2 = new Fl_SteinerKnob(481, 50,
                             34, 34, _("cut")); o->labelsize(8); o->value(50);
                                o->maximum(10000);
                             o->callback((Fl_Callback*)callback);*/
                            o->argument(33);
                            Knob[i][o->argument()] = o;
                        }
                        {
                            Mw_Dial *o = f1q2 = new Mw_Dial(531, 32, 25, 25, _("q"));
                            o->labelsize(8);

                            o->argument(34);
                            o->minimum(0.9);
                            o->value(0.5);
                            o->maximum(0.01);
                            o->callback((Fl_Callback *)callback);
                            Knob[i][o->argument()] = o;
                        }
                        {
                            Mw_Dial *o = f1vol2 = new Mw_Dial(541, 70, 20, 20, _("vol"));
                            o->labelsize(8);
                            o->labelsize(8);
                            o->minimum(-1);
                            o->value(0.5);
                            o->maximum(1);
                            o->argument(35);
                            o->callback((Fl_Callback *)callback);
                            Knob[i][o->argument()] = o;
                        }
                        {
                            Fl_Value_Input *o = new Fl_Value_Input(412, 100, 38, 15);
                            o->box(FL_ROUNDED_BOX);
                            o->labelsize(8);
                            o->textsize(8);
                            o->maximum(10000);
                            o->step(0.01);
                            o->value(200);
                            o->argument(30);
                            o->callback((Fl_Callback *)cutoffCallback);
                            miniDisplay[i][4] = o;
                        }
                        {
                            Fl_Value_Input *o = new Fl_Value_Input(528, 100, 38, 15);
                            o->box(FL_ROUNDED_BOX);
                            o->labelsize(8);
                            o->textsize(8);
                            o->maximum(10000);
                            o->step(0.01);
                            o->value(20);
                            o->argument(33);
                            o->callback((Fl_Callback *)cutoffCallback);
                            miniDisplay[i][5] = o;
                        }
                        /*
                            { Fl_Button* o = new Fl_Button(426, 35, 45, 15,
                           _("copy ->")); o->box(FL_BORDER_BOX); o->labelsize(8);
                            }
                            { Fl_Button* o = new Fl_Button(426, 59, 45, 15, _("<-
                           copy")); o->box(FL_BORDER_BOX); o->labelsize(8);
                            }*/
                        o->end();
                    }
                    {
                        Mw_Dial *o = new Mw_Dial(418, 360, 60, 57, _("morph"));
                        o->type(1);
                        o->labelsize(8);
                        o->maximum(0.5f);
                        o->argument(56);
                        o->callback((Fl_Callback *)callback);
                        Knob[i][o->argument()] = o;
                    }
                    {
                        Mw_Dial *o = new Mw_Dial(326, 392, 25, 25, _("amount"));
                        o->labelsize(8);
                        o->minimum(-2);
                        o->maximum(2);
                        o->argument(38);
                        o->callback((Fl_Callback *)callback);
                        Knob[i][o->argument()] = o;
                    }
                    {
                        Fl_Choice *o = new Fl_Choice(325, 366, 85, 15, _("morph mod 1"));
                        o->down_box(FL_BORDER_BOX);
                        o->labelsize(8);
                        o->textsize(8);
                        o->align(FL_ALIGN_TOP_LEFT);
                        o->menu(menu_amod);
                        o->argument(10);
                        o->callback((Fl_Callback *)choicecallback);
                        auswahl[i][o->argument()] = o;
                    }
                    {
                        Mw_Dial *o = new Mw_Dial(551, 392, 25, 25, _("amount"));
                        o->labelsize(8);
                        o->argument(48);
                        o->minimum(-2);
                        o->maximum(2);
                        o->callback((Fl_Callback *)callback);
                        Knob[i][o->argument()] = o;
                    }
                    {
                        Fl_Choice *o = new Fl_Choice(492, 366, 85, 15, _("morph mod 2"));
                        o->down_box(FL_BORDER_BOX);
                        o->labelsize(8);
                        o->textsize(8);
                        o->align(FL_ALIGN_TOP_LEFT);
                        o->menu(menu_amod);
                        o->argument(11);
                        o->callback((Fl_Callback *)choicecallback);
                        auswahl[i][o->argument()] = o;
                    }
                    {
                        Fl_Group *o = new Fl_Group(330, 132, 239, 92, _("filter 2"));
                        o->box(FL_ROUNDED_FRAME);
                        o->color(FL_FOREGROUND_COLOR);
                        o->labelsize(8);
                        {
                            Fl_Positioner *o = new Fl_Positioner(340, 135, 70, 79, _("cut"));
                            o->xbounds(0, 7000);
                            o->ybounds(499, 0);
                            o->selection_color(0);
                            o->box(FL_BORDER_BOX);
                            o->xstep(500);
                            o->labelsize(8);
                            /*Fl_Dial* o = f1cut1 = new Fl_SteinerKnob(344, 155,
                              34, 34, _("cut")); o->labelsize(8); o->labelsize(8);
                               o->value(50);
                              o->maximum(10000);*/
                            o->argument(40);
                            Knob[i][o->argument()] = o;
                            o->callback((Fl_Callback *)callback);
                        }
                        {
                            Mw_Dial *o = f1q1 = new Mw_Dial(415, 137, 25, 25, _("q"));
                            o->labelsize(8);
                            o->argument(41);
                            o->minimum(0.9);
                            o->maximum(0.001);
                            o->value(0.5);
                            o->callback((Fl_Callback *)callback);
                            Knob[i][o->argument()] = o;
                        }
                        {
                            Mw_Dial *o = f1vol1 = new Mw_Dial(425, 174, 20, 20, _("vol"));
                            o->labelsize(8);
                            o->argument(42);
                            o->callback((Fl_Callback *)callback);
                            o->minimum(-1);
                            o->value(0);
                            o->maximum(1);
                            Knob[i][o->argument()] = o;
                        }
                        {
                            Fl_Positioner *o = new Fl_Positioner(456, 135, 70, 79, _("cut"));
                            o->xbounds(0, 7000);
                            o->ybounds(499, 0);
                            o->selection_color(0);
                            o->box(FL_BORDER_BOX);
                            o->xstep(500);
                            o->labelsize(8);

                            /*Fl_Dial* o = f1cut2 = new Fl_SteinerKnob(481, 154,
                              34, 34, _("cut")); o->labelsize(8); o->labelsize(8);
                               o->value(50);
                              o->maximum(10000);*/
                            o->argument(43);
                            Knob[i][o->argument()] = o;
                            o->callback((Fl_Callback *)callback);
                        }
                        {
                            Mw_Dial *o = f1q2 = new Mw_Dial(531, 136, 25, 25, _("q"));
                            o->labelsize(8);
                            o->labelsize(8);
                            o->argument(44);
                            o->minimum(0.9);
                            o->value(0.5);
                            o->maximum(0.001);
                            o->callback((Fl_Callback *)callback);
                            Knob[i][o->argument()] = o;
                        }
                        {
                            Mw_Dial *o = f1vol2 = new Mw_Dial(541, 174, 20, 20, _("vol"));
                            o->labelsize(8);
                            o->labelsize(8);
                            o->argument(45);
                            o->maximum(1);
                            o->callback((Fl_Callback *)callback);
                            o->minimum(-1);
                            o->value(0);
                            Knob[i][o->argument()] = o;
                        }
                        {
                            Fl_Value_Input *o = new Fl_Value_Input(412, 204, 38, 15);
                            o->box(FL_ROUNDED_BOX);
                            o->labelsize(8);
                            o->maximum(10000);
                            o->argument(40);
                            o->step(0.01);
                            o->callback((Fl_Callback *)cutoffCallback);
                            o->textsize(8);
                            miniDisplay[i][6] = o;
                        }
                        {
                            Fl_Value_Input *o = new Fl_Value_Input(528, 204, 38, 15);
                            o->box(FL_ROUNDED_BOX);
                            o->labelsize(8);
                            o->argument(43);
                            o->maximum(10000);
                            o->step(0.01);
                            o->callback((Fl_Callback *)cutoffCallback);
                            o->textsize(8);
                            miniDisplay[i][7] = o;
                        }
                        /*
                            { Fl_Button* o = new Fl_Button(426, 139, 45, 15,
                          _("copy ->")); o->box(FL_BORDER_BOX); o->labelsize(8);
                              o->argument(21);
                            //o->callback((Fl_Callback*)copycallback);
                            }
                            { Fl_Button* o = new Fl_Button(426, 163, 45, 15, _("<-
                          copy")); o->box(FL_BORDER_BOX); o->labelsize(8);
                              o->argument(22);
                          //o->callback((Fl_Callback*)copycallback);
                            }*/
                        o->end();
                    }
                    {
                        Fl_Group *o = new Fl_Group(330, 238, 239, 92, _("filter 3"));
                        o->box(FL_ROUNDED_FRAME);
                        o->color(FL_FOREGROUND_COLOR);
                        o->labelsize(8);
                        {
                            Fl_Positioner *o = new Fl_Positioner(340, 241, 70, 79, _("cut"));
                            o->xbounds(0, 7000);
                            o->ybounds(499, 0);
                            o->selection_color(0);
                            o->box(FL_BORDER_BOX);
                            o->xstep(500);
                            o->labelsize(8);
                            /* Fl_Dial* o = f1cut1 = new Fl_SteinerKnob(344,
                              261, 34, 34, _("cut")); o->labelsize(8);
                              o->value(50);
                              o->maximum(10000);*/
                            o->argument(50);
                            o->callback((Fl_Callback *)callback);
                            Knob[i][o->argument()] = o;
                        }
                        {
                            Mw_Dial *o = f1q1 = new Mw_Dial(415, 243, 25, 25, _("q"));
                            o->labelsize(8);
                            o->argument(51);
                            o->minimum(0.9);
                            o->maximum(0.001);
                            o->value(0.5);
                            o->callback((Fl_Callback *)callback);
                            Knob[i][o->argument()] = o;
                        }
                        {
                            Mw_Dial *o = f1vol1 = new Mw_Dial(425, 280, 20, 20, _("vol"));
                            o->labelsize(8);
                            o->argument(52);
                            o->maximum(1);
                            o->minimum(-1);
                            o->value(0);
                            o->callback((Fl_Callback *)callback);
                            Knob[i][o->argument()] = o;
                        }
                        {
                            Fl_Positioner *o = new Fl_Positioner(456, 241, 70, 79, _("cut"));
                            o->xbounds(0, 7000);
                            o->ybounds(499, 0);
                            o->selection_color(0);
                            o->box(FL_BORDER_BOX);
                            o->xstep(500);
                            o->labelsize(8);
                            /*Fl_Dial* o = f1cut2 = new Fl_SteinerKnob(481, 260,
                              34, 34, _("cut")); o->labelsize(8); o->value(50);
                              o->maximum(10000);*/
                            o->argument(53);
                            Knob[i][o->argument()] = o;
                            o->callback((Fl_Callback *)callback);
                        }
                        {
                            Mw_Dial *o = f1q2 = new Mw_Dial(531, 242, 25, 25, _("q"));
                            o->labelsize(8);
                            o->argument(54);
                            o->minimum(0.9);
                            o->value(0.5);
                            o->maximum(0.001);
                            o->callback((Fl_Callback *)callback);
                            Knob[i][o->argument()] = o;
                        }
                        {
                            Mw_Dial *o = f1vol2 = new Mw_Dial(541, 280, 20, 20, _("vol"));
                            o->labelsize(8);
                            o->labelsize(8);
                            o->argument(55);
                            o->maximum(1);
                            o->minimum(-1);
                            o->value(0);
                            o->callback((Fl_Callback *)callback);
                            Knob[i][o->argument()] = o;
                        }
                        {
                            Fl_Value_Input *o = new Fl_Value_Input(412, 310, 38, 15);
                            o->box(FL_ROUNDED_BOX);
                            o->labelsize(8);
                            o->maximum(10000);
                            o->step(0.01);
                            o->argument(50);
                            o->callback((Fl_Callback *)cutoffCallback);
                            o->textsize(8);
                            miniDisplay[i][8] = o;
                        }
                        {
                            Fl_Value_Input *o = new Fl_Value_Input(528, 310, 38, 15);
                            o->box(FL_ROUNDED_BOX);
                            o->labelsize(8);
                            o->maximum(10000);
                            o->argument(53);
                            o->step(0.01);
                            o->callback((Fl_Callback *)cutoffCallback);
                            o->textsize(8);
                            miniDisplay[i][9] = o;
                        }
                        /*
                            { Fl_Button* o = new Fl_Button(426, 245, 45, 15,
                           _("copy ->")); o->box(FL_BORDER_BOX); o->labelsize(8);
                            }
                            { Fl_Button* o = new Fl_Button(426, 269, 45, 15, _("<-
                           copy")); o->box(FL_BORDER_BOX); o->labelsize(8);
                            }*/
                        o->end();
                    }
                    {
                        Fl_Button *o = new Fl_Button(480, 430, 62, 15, _("clear filter"));
                        o->tooltip(_("reset the filter"));
                        o->box(FL_BORDER_BOX);
                        o->labelsize(8);
                        o->labelcolor((Fl_Color)186);
                        o->argument(0);
                        o->callback((Fl_Callback *)callback);
                    }
                    o->end();
                }
                {
                    Fl_Group *o = new Fl_Group(595, 17, 225, 433);
                    o->box(FL_ROUNDED_FRAME);
                    o->color(FL_BACKGROUND2_COLOR);
                    {
                        Fl_Box *d = new Fl_Box(595, 225, 210, 432, _("modulators"));
                        d->labelsize(8);
                        d->labelcolor(FL_BACKGROUND2_COLOR);
                    }
                    // ----------- knobs for envelope generator 1 ---------------
                    {
                        Fl_Group *o = new Fl_Group(608, 31, 200, 45, _("EG 1"));
                        o->box(FL_ROUNDED_FRAME);
                        o->color(FL_FOREGROUND_COLOR);
                        o->labelsize(8);
                        {
                            Mw_Dial *o = new Mw_Dial(618, 37, 25, 25, _("A"));
                            o->labelsize(8);
                            o->argument(60);
                            o->minimum(0.2);
                            o->maximum(0.01);
                            o->callback((Fl_Callback *)callback);
                            Knob[i][o->argument()] = o;
                        }
                        {
                            Mw_Dial *o = new Mw_Dial(648, 37, 25, 25, _("D"));
                            o->labelsize(8);
                            o->argument(61);
                            o->minimum(0.15);
                            o->maximum(0.01);

                            o->callback((Fl_Callback *)callback);
                            Knob[i][o->argument()] = o;
                        }

                        {
                            Mw_Dial *o = new Mw_Dial(678, 37, 25, 25, _("S"));
                            o->labelsize(8);
                            o->argument(62);
                            // o->minimum(0);
                            // o->maximum(0.001);
                            o->callback((Fl_Callback *)callback);
                            Knob[i][o->argument()] = o;
                        }
                        {
                            Mw_Dial *o = new Mw_Dial(708, 37, 25, 25, _("R"));
                            o->labelsize(8);
                            o->argument(63);
                            o->minimum(0.15);
                            o->maximum(0.02);
                            o->callback((Fl_Callback *)callback);
                            Knob[i][o->argument()] = o;
                        }
                        {
                            Fl_Light_Button *o =
                                new Fl_Light_Button(744, 42, 55, 15, _("repeat"));
                            o->box(FL_BORDER_BOX);
                            o->selection_color((Fl_Color)89);
                            o->labelsize(8);
                            o->argument(64);
                            o->callback((Fl_Callback *)callback);
                            Knob[i][o->argument()] = o;
                        }
                        o->end();
                    }
                    // ----------- knobs for envelope generator 2 ---------------
                    {
                        Fl_Group *o = new Fl_Group(608, 90, 200, 45, _("EG 2"));
                        o->box(FL_ROUNDED_FRAME);
                        o->color(FL_FOREGROUND_COLOR);
                        o->labelsize(8);
                        {
                            Mw_Dial *o = new Mw_Dial(618, 96, 25, 25, _("A"));
                            o->labelsize(8);
                            o->argument(65);
                            o->minimum(0.2);
                            o->maximum(0.01);
                            o->callback((Fl_Callback *)callback);
                            Knob[i][o->argument()] = o;
                        }
                        {
                            Mw_Dial *o = new Mw_Dial(648, 96, 25, 25, _("D"));
                            o->labelsize(8);
                            o->argument(66);
                            o->minimum(0.15);
                            o->maximum(0.01);
                            o->callback((Fl_Callback *)callback);
                            Knob[i][o->argument()] = o;
                        }
                        {
                            Mw_Dial *o = new Mw_Dial(678, 96, 25, 25, _("S"));
                            o->labelsize(8);
                            o->argument(67);
                            o->callback((Fl_Callback *)callback);
                            Knob[i][o->argument()] = o;
                        }
                        {
                            Mw_Dial *o = new Mw_Dial(708, 96, 25, 25, _("R"));
                            o->labelsize(8);
                            o->argument(68);
                            o->minimum(0.15);
                            o->maximum(0.02);
                            o->callback((Fl_Callback *)callback);
                            Knob[i][o->argument()] = o;
                        }
                        {
                            Fl_Light_Button *o =
                                new Fl_Light_Button(744, 101, 55, 15, _("repeat"));
                            o->box(FL_BORDER_BOX);
                            o->selection_color((Fl_Color)89);
                            o->labelsize(8);
                            o->argument(69);
                            o->callback((Fl_Callback *)callback);
                            Knob[i][o->argument()] = o;
                        }
                        o->end();
                    }
                    // ----------- knobs for envelope generator 3 ---------------
                    {
                        Fl_Group *o = new Fl_Group(608, 147, 200, 45, _("EG 3"));
                        o->box(FL_ROUNDED_FRAME);
                        o->color(FL_FOREGROUND_COLOR);
                        o->labelsize(8);
                        {
                            Mw_Dial *o = new Mw_Dial(618, 153, 25, 25, _("A"));
                            o->labelsize(8);
                            o->argument(70);
                            o->minimum(0.2);
                            o->maximum(0.01);
                            o->callback((Fl_Callback *)callback);
                            Knob[i][o->argument()] = o;
                        }
                        {
                            Mw_Dial *o = new Mw_Dial(648, 153, 25, 25, _("D"));
                            o->labelsize(8);
                            o->argument(71);
                            o->minimum(0.15);
                            o->maximum(0.01);
                            o->callback((Fl_Callback *)callback);
                            Knob[i][o->argument()] = o;
                        }
                        {
                            Mw_Dial *o = new Mw_Dial(678, 153, 25, 25, _("S"));
                            o->labelsize(8);
                            o->argument(72);
                            o->callback((Fl_Callback *)callback);
                            Knob[i][o->argument()] = o;
                        }
                        {
                            Mw_Dial *o = new Mw_Dial(708, 153, 25, 25, _("R"));
                            o->labelsize(8);
                            o->argument(73);
                            o->minimum(0.15);
                            o->maximum(0.02);
                            o->callback((Fl_Callback *)callback);
                            Knob[i][o->argument()] = o;
                        }
                        {
                            Fl_Light_Button *o =
                                new Fl_Light_Button(744, 158, 55, 15, _("repeat"));
                            o->box(FL_BORDER_BOX);
                            o->selection_color((Fl_Color)89);
                            o->labelsize(8);
                            o->argument(74);
                            o->callback((Fl_Callback *)callback);
                            Knob[i][o->argument()] = o;
                        }
                        o->end();
                    }
                    // ----------- knobs for envelope generator 4 ---------------
                    {
                        Fl_Group *o = new Fl_Group(608, 204, 200, 45, _("EG 4"));
                        o->box(FL_ROUNDED_FRAME);
                        o->color(FL_FOREGROUND_COLOR);
                        o->labelsize(8);
                        {
                            Mw_Dial *o = new Mw_Dial(618, 210, 25, 25, _("A"));
                            o->labelsize(8);
                            o->argument(75);
                            o->minimum(0.2);
                            o->maximum(0.01);
                            o->callback((Fl_Callback *)callback);
                            Knob[i][o->argument()] = o;
                        }
                        {
                            Mw_Dial *o = new Mw_Dial(648, 210, 25, 25, _("D"));
                            o->labelsize(8);
                            o->argument(76);
                            o->minimum(0.15);
                            o->maximum(0.01);
                            o->callback((Fl_Callback *)callback);
                            Knob[i][o->argument()] = o;
                        }
                        {
                            Mw_Dial *o = new Mw_Dial(678, 210, 25, 25, _("S"));
                            o->labelsize(8);
                            o->argument(77);
                            o->callback((Fl_Callback *)callback);
                            Knob[i][o->argument()] = o;
                        }
                        {
                            Mw_Dial *o = new Mw_Dial(708, 210, 25, 25, _("R"));
                            o->labelsize(8);
                            o->argument(78);
                            o->minimum(0.15);
                            o->maximum(0.02);
                            o->callback((Fl_Callback *)callback);
                            Knob[i][o->argument()] = o;
                        }
                        {
                            Fl_Light_Button *o =
                                new Fl_Light_Button(744, 215, 55, 15, _("repeat"));
                            o->box(FL_BORDER_BOX);
                            o->selection_color((Fl_Color)89);
                            o->labelsize(8);
                            o->argument(79);
                            o->callback((Fl_Callback *)callback);
                            Knob[i][o->argument()] = o;
                        }
                        o->end();
                    }
                    // ----------- knobs for envelope generator 5 ---------------
                    {
                        Fl_Group *o = new Fl_Group(608, 263, 200, 45, _("EG 5"));
                        o->box(FL_ROUNDED_FRAME);
                        o->color(FL_FOREGROUND_COLOR);
                        o->labelsize(8);
                        {
                            Mw_Dial *o = new Mw_Dial(618, 269, 25, 25, _("A"));
                            o->labelsize(8);
                            o->argument(80);
                            o->minimum(0.2);
                            o->maximum(0.01);
                            o->callback((Fl_Callback *)callback);
                            Knob[i][o->argument()] = o;
                        }
                        {
                            Mw_Dial *o = new Mw_Dial(648, 269, 25, 25, _("D"));
                            o->labelsize(8);
                            o->argument(81);
                            o->minimum(0.15);
                            o->maximum(0.01);
                            o->callback((Fl_Callback *)callback);
                            Knob[i][o->argument()] = o;
                        }
                        {
                            Mw_Dial *o = new Mw_Dial(678, 269, 25, 25, _("S"));
                            o->labelsize(8);
                            o->argument(82);
                            o->callback((Fl_Callback *)callback);
                            Knob[i][o->argument()] = o;
                        }
                        {
                            Mw_Dial *o = new Mw_Dial(708, 269, 25, 25, _("R"));
                            o->labelsize(8);
                            o->argument(83);
                            o->minimum(0.15);
                            o->maximum(0.02);
                            o->callback((Fl_Callback *)callback);
                            Knob[i][o->argument()] = o;
                        }
                        {
                            Fl_Light_Button *o =
                                new Fl_Light_Button(744, 274, 55, 15, _("repeat"));
                            o->box(FL_BORDER_BOX);
                            o->selection_color((Fl_Color)89);
                            o->labelsize(8);
                            o->argument(84);
                            o->callback((Fl_Callback *)callback);
                            Knob[i][o->argument()] = o;
                        }
                        o->end();
                    }
                    // ------ knobs for envelope generator 6 -----
                    {
                        Fl_Group *o = new Fl_Group(608, 324, 200, 45, _("EG 6"));
                        o->box(FL_ROUNDED_FRAME);
                        o->color(FL_FOREGROUND_COLOR);
                        o->labelsize(8);
                        {
                            Mw_Dial *o = new Mw_Dial(618, 330, 25, 25, _("A"));
                            o->labelsize(8);
                            o->argument(85);
                            o->minimum(0.2);
                            o->maximum(0.01);
                            Knob[i][o->argument()] = o;

                            o->callback((Fl_Callback *)callback);
                        }
                        {
                            Mw_Dial *o = new Mw_Dial(648, 330, 25, 25, _("D"));
                            o->labelsize(8);
                            o->argument(86);
                            o->minimum(0.15);
                            o->maximum(0.01);
                            o->callback((Fl_Callback *)callback);
                            Knob[i][o->argument()] = o;
                        }
                        {
                            Mw_Dial *o = new Mw_Dial(678, 330, 25, 25, _("S"));
                            o->labelsize(8);
                            o->argument(87);
                            o->callback((Fl_Callback *)callback);
                            Knob[i][o->argument()] = o;
                        }
                        {
                            Mw_Dial *o = new Mw_Dial(708, 330, 25, 25, _("R"));
                            o->labelsize(8);
                            o->argument(88);
                            o->minimum(0.15);
                            o->maximum(0.02);
                            o->callback((Fl_Callback *)callback);
                            Knob[i][o->argument()] = o;
                        }
                        {
                            Fl_Light_Button *o =
                                new Fl_Light_Button(744, 335, 55, 15, _("repeat"));
                            o->box(FL_BORDER_BOX);
                            o->selection_color((Fl_Color)89);
                            o->labelsize(8);
                            o->argument(89);
                            o->callback((Fl_Callback *)callback);
                            Knob[i][o->argument()] = o;
                        }
                        o->end();
                    }
                    {
                        Fl_Group *o = new Fl_Group(608, 380, 200, 54, _("mod osc"));
                        o->box(FL_ROUNDED_FRAME);
                        o->color(FL_FOREGROUND_COLOR);
                        o->labelsize(8);

                        {
                            Fl_Positioner *o = new Fl_Positioner(620, 384, 50, 40, _("tune"));
                            o->xbounds(0, 128);
                            o->ybounds(0.99, 0);
                            o->box(FL_BORDER_BOX);
                            o->xstep(1);
                            o->labelsize(8);
                            o->argument(90);
                            o->selection_color(0);
                            o->callback((Fl_Callback *)callback);
                            Knob[i][o->argument()] = o;
                            /*Fl_Dial* o = new Fl_SteinerKnob(627, 392, 34, 34,
                               _("frequency")); o->labelsize(8);o->argument(90);
                                o->callback((Fl_Callback*)callback);
                                o->maximum(500); */
                        }
                        {
                            Fl_Choice *o = new Fl_Choice(680, 397, 120, 15, _("waveform"));
                            o->box(FL_BORDER_BOX);
                            o->down_box(FL_BORDER_BOX);
                            o->labelsize(8);
                            o->textsize(8);
                            o->align(FL_ALIGN_TOP_LEFT);
                            o->menu(menu_wave);
                            o->argument(12);
                            o->callback((Fl_Callback *)choicecallback);
                            auswahl[i][o->argument()] = o;
                        }
                        {
                            Fl_Value_Input *o = new Fl_Value_Input(690, 415, 38, 15);  // frequency display for modulation oscillator
                            o->box(FL_ROUNDED_BOX);
                            o->labelsize(8);
                            o->maximum(10000);
                            o->step(0.001);
                            o->textsize(8);
                            o->callback((Fl_Callback *)tuneCallback);
                            o->argument(90);
                            miniDisplay[i][10] = o;
                        }
                        o->end();
                    }
                    o->end();
                }


                {
                    Fl_Group *d = new Fl_Group(5, 461, 680, 45, _("memory"));
                    d->box(FL_ROUNDED_FRAME);
                    d->color(FL_BACKGROUND2_COLOR);
                    d->labelsize(8);
                    d->labelcolor(FL_BACKGROUND2_COLOR);
                    d->begin();
                    /* { Fl_Button* o = new Fl_Button(191, 473, 50, 19, _("create
                     bank")); o->tooltip(_("create a new bank after current one"));
                       o->box(FL_BORDER_BOX);
                       o->labelsize(8);
                     }
                     { Fl_Button* o = new Fl_Button(26, 476, 53, 14, _("delete
                     bank")); o->tooltip(_("delete a whole bank of sounds!"));
                       o->box(FL_BORDER_BOX);
                       o->labelsize(8);
                       o->labelcolor((Fl_Color)1);
                     }
                     { Fl_Button* o = new Fl_Button(732, 475, 59, 14, _("delete
                     sound")); o->tooltip(_("delete current sound"));
                       o->box(FL_BORDER_BOX);
                       o->labelsize(8);
                       o->labelcolor((Fl_Color)1);
                     }*/
                    {
                        Fl_Input *o = new Fl_Input(274, 471, 150, 14, _("sound"));
                        o->box(FL_BORDER_BOX);
                        // o->down_box(FL_BORDER_FRAME);
                        // o->color(FL_FOREGROUND_COLOR);
                        // o->selection_color(FL_FOREGROUND_COLOR);
                        o->labelsize(8);
                        o->textsize(8);
                        // o->menubutton()->textsize(8);
                        // o->menubutton()->type(Fl_Menu_Button::POPUP1);
                        o->align(FL_ALIGN_TOP_LEFT);
                        soundchoice[i] = o;
                        schoice[i] = o;
                        d->add(o);
                        // o->callback((Fl_Callback*)chooseCallback,NULL);
                    }
                    {
                        Mw_Roller *o = new Mw_Roller(274, 487, 150, 14);
                        o->type(FL_HORIZONTAL);
                        o->tooltip(_("roll the list of sounds"));
                        o->minimum(0);
                        o->maximum(511);
                        o->step(1);
                        // o->slider_size(0.04);
                        o->box(FL_BORDER_FRAME);
                        o->mouse_wheel_steps(512);
                        Rollers[i] = o;
                        o->callback((Fl_Callback *)rollerCallback, NULL);
                    }

                    {
                        Fl_Button *o = new Fl_Button(516, 465, 59, 19, _("store sound"));
                        o->tooltip(_("store this sound on current entry"));
                        o->box(FL_BORDER_BOX);
                        o->labelsize(8);
                        o->labelcolor((Fl_Color)1);
                        o->callback((Fl_Callback *)storesound, soundchoice[i]);
                    }
                    {
                        Fl_Button *o = new Fl_Button(436, 465, 70, 19, _("load sound"));
                        o->tooltip(_("actually load the chosen sound"));
                        o->box(FL_BORDER_BOX);
                        o->labelsize(8);
                        o->labelcolor((Fl_Color)186);
                        o->callback((Fl_Callback *)loadsound, soundchoice[i]);
                    }
                    {
                        Fl_Value_Output *o =
                            new Fl_Value_Output(490, 488, 20, 15, _("memory"));
                        o->box(FL_ROUNDED_BOX);
                        o->color(FL_BLACK);
                        o->labelsize(8);
                        o->maximum(512);
                        o->textsize(8);
                        o->textcolor(FL_RED);
                        // std::string tooltip;
                        // tooltip = astrprintf(_("accept Midi Program Change on
                        // channel %i"),i); o->tooltip(tooltip.c_str());
                        memDisplay[i] = o;
                    }
                    {
                        Fl_Button *o = new Fl_Button(600, 469, 70, 12, _("import sound"));
                        o->tooltip(_("import single sound to dialed memory slot, "
                                     "you need to load it for playing"));
                        o->box(FL_BORDER_BOX);
                        o->labelsize(8);
                        // o->labelcolor((Fl_Color)1);
                        o->callback((Fl_Callback *)importPressed, soundchoice[i]);
                    }

                    {
                        Fl_Button *o = new Fl_Button(600, 485, 70, 12, _("export sound"));
                        o->tooltip(_("export sound data of dialed memory slot"));
                        o->box(FL_BORDER_BOX);
                        o->labelsize(8);
                        // o->labelcolor((Fl_Color)186);
                        o->callback((Fl_Callback *)exportPressed, soundchoice[i]);
                    }
                    /*{ Fl_Input_Choice* o = new Fl_Input_Choice(83, 476, 105,
                    14, _("bank")); o->box(FL_BORDER_FRAME);
                      o->down_box(FL_BORDER_FRAME);
                      o->color(FL_FOREGROUND_COLOR);
                      o->selection_color(FL_FOREGROUND_COLOR);
                      o->labelsize(8);
                      o->textsize(8);
                      o->align(FL_ALIGN_TOP_LEFT);
                    }*/
                    d->end();
                }
                {
                    Fl_Group *o = new Fl_Group(825, 17, 160, 223);
                    o->box(FL_ROUNDED_FRAME);
                    o->color(FL_BACKGROUND2_COLOR);
                    {
                        Fl_Box *d = new Fl_Box(825, 164, 160, 135, _("amp"));
                        d->labelsize(8);
                        d->labelcolor(FL_BACKGROUND2_COLOR);
                    }
                    // amplitude envelope
                    {
                        Mw_Dial *o = new Mw_Dial(844, 103, 25, 25, _("A"));
                        o->labelsize(8);
                        o->argument(102);
                        o->minimum(0.20);
                        o->maximum(0.01);
                        o->callback((Fl_Callback *)callback);
                        Knob[i][o->argument()] = o;
                    }
                    {
                        Mw_Dial *o = new Mw_Dial(874, 103, 25, 25, _("D"));
                        o->labelsize(8);
                        o->argument(103);
                        o->minimum(0.15);
                        o->maximum(0.01);
                        o->callback((Fl_Callback *)callback);
                        Knob[i][o->argument()] = o;
                    }
                    {
                        Mw_Dial *o = new Mw_Dial(904, 103, 25, 25, _("S"));
                        o->labelsize(8);
                        o->argument(104);
                        o->callback((Fl_Callback *)callback);
                        Knob[i][o->argument()] = o;
                    }
                    {
                        Mw_Dial *o = new Mw_Dial(934, 103, 25, 25, _("R"));
                        o->labelsize(8);
                        o->argument(105);
                        o->minimum(0.15);
                        o->maximum(0.02);
                        o->callback((Fl_Callback *)callback);
                        Knob[i][o->argument()] = o;
                    }
                    {
                        Mw_Dial *o = new Mw_Dial(930, 58, 25, 25, _("mod amount"));
                        o->labelsize(8);
                        o->argument(100);
                        o->callback((Fl_Callback *)callback);
                        Knob[i][o->argument()] = o;
                    }
                    {
                        Fl_Choice *o = new Fl_Choice(844, 35, 120, 15, _("amp modulator"));
                        o->box(FL_BORDER_BOX);
                        o->down_box(FL_BORDER_BOX);
                        o->labelsize(8);
                        o->textsize(8);
                        o->align(FL_ALIGN_TOP_LEFT);
                        o->menu(menu_amod);
                        o->argument(13);
                        o->callback((Fl_Callback *)choicecallback);
                        auswahl[i][o->argument()] = o;
                    }
                    /*
                    { Fl_Counter* o = new Fl_Counter(844, 151, 115, 14,
                    _("sound")); o->type(1); o->box(FL_BORDER_BOX);
                      o->labelsize(8);
                      o->minimum(0);
                      o->maximum(7);
                      o->step(1);
                      o->value(0);
                      o->textsize(8);
                    //  o->callback((Fl_Callback*)voicecallback,soundchoice[0]);
                    }
                    { Fl_Counter* o = new Fl_Counter(844, 181, 115, 14,
                    _("midichannel")); o->type(1); o->box(FL_BORDER_BOX);
                      o->labelsize(8);
                      o->minimum(1);
                      o->maximum(16);
                      o->step(1);
                      o->value(1);
                      o->textsize(8);
                    }*/
                    {
                        Mw_Dial *o = new Mw_Dial(844, 150, 25, 25, _("id vol"));
                        o->labelsize(8);
                        o->argument(101);
                        o->minimum(0);
                        o->maximum(2);
                        o->color(fl_rgb_color(190, 160, 255));
                        o->value(0);
                        o->callback((Fl_Callback *)callback);
                        Knob[i][o->argument()] = o;
                    }
                    {
                        Mw_Dial *o = new Mw_Dial(874, 150, 25, 25, _("aux 1"));
                        o->labelsize(8);
                        o->argument(108);
                        o->minimum(0);
                        o->maximum(2);
                        o->color(fl_rgb_color(140, 140, 255));
                        o->value(0);
                        o->callback((Fl_Callback *)callback);
                        Knob[i][o->argument()] = o;
                    }
                    {
                        Mw_Dial *o = new Mw_Dial(904, 150, 25, 25, _("aux 2"));
                        o->labelsize(8);
                        o->argument(109);
                        o->minimum(0);
                        o->color(fl_rgb_color(140, 140, 255));
                        o->maximum(2);
                        o->value(0);
                        o->callback((Fl_Callback *)callback);
                        Knob[i][o->argument()] = o;
                    }
                    {
                        Mw_Dial *o = new Mw_Dial(934, 150, 25, 25, _("mix vol"));
                        o->labelsize(8);
                        o->argument(106);
                        o->minimum(0);
                        o->maximum(2);
                        o->color(fl_rgb_color(170, 140, 255));
                        o->value(0);
                        o->callback((Fl_Callback *)callback);
                        Knob[i][o->argument()] = o;
                    }
                    {
                        Fl_Slider *o = new Fl_Slider(864, 200, 80, 10, _("mix pan"));
                        o->labelsize(8);
                        o->box(FL_BORDER_BOX);
                        o->argument(107);
                        o->minimum(0);
                        o->maximum(1);
                        o->color(fl_rgb_color(170, 140, 255));
                        o->value(0.0);
                        o->type(FL_HORIZONTAL);
                        o->callback((Fl_Callback *)callback);
                        Knob[i][o->argument()] = o;
                    }
                    o->end();
                }
                {
                    Fl_Group *o = new Fl_Group(825, 250, 160, 140);
                    o->box(FL_ROUNDED_FRAME);
                    o->color(FL_BACKGROUND2_COLOR);
                    {
                        Fl_Box *d = new Fl_Box(825, 313, 160, 135, _("delay"));
                        d->labelsize(8);
                        d->labelcolor(FL_BACKGROUND2_COLOR);
                    }
                    {
                        Mw_Dial *o = new Mw_Dial(930, 288, 25, 25, _("mod amount"));
                        o->labelsize(8);
                        o->argument(110);
                        o->callback((Fl_Callback *)callback);
                        Knob[i][o->argument()] = o;
                    }
                    {
                        Fl_Choice *o = new Fl_Choice(844, 265, 120, 15, _("time modulator"));
                        o->box(FL_BORDER_BOX);
                        o->down_box(FL_BORDER_BOX);
                        o->labelsize(8);
                        o->textsize(8);
                        o->align(FL_ALIGN_TOP_LEFT);
                        o->menu(menu_amod);
                        o->argument(14);
                        o->callback((Fl_Callback *)choicecallback);
                        auswahl[i][o->argument()] = o;
                    }
                    {
                        Mw_Dial *o = new Mw_Dial(844, 288, 25, 25, _("delay time"));
                        o->labelsize(8);
                        o->argument(111);
                        o->callback((Fl_Callback *)callback);
                        Knob[i][o->argument()] = o;
                    }
                    {
                        Mw_Dial *o = new Mw_Dial(874, 330, 25, 25, _("feedback"));
                        o->labelsize(8);
                        o->argument(112);
                        o->callback((Fl_Callback *)callback);
                        Knob[i][o->argument()] = o;
                    }
                    {
                        Mw_Dial *o = new Mw_Dial(950, 330, 25, 25, _("volume"));
                        o->labelsize(8);
                        o->argument(113);
                        o->callback((Fl_Callback *)callback);
                        Knob[i][o->argument()] = o;
                    }
                    o->end();
                }
                {
                    Mw_Dial *o = new Mw_Dial(295, 191, 25, 25, _("osc1  vol"));
                    o->labelsize(8);
                    o->align(FL_ALIGN_TOP);
                    o->argument(14);
                    o->callback((Fl_Callback *)callback);
                    Knob[i][o->argument()] = o;
                }
                {
                    Mw_Dial *o = new Mw_Dial(295, 252, 25, 25, _("osc2  vol"));
                    o->labelsize(8);
                    o->argument(29);
                    o->callback((Fl_Callback *)callback);
                    Knob[i][o->argument()] = o;
                }
                {
                    Mw_Dial *o = new Mw_Dial(950, 228, 25, 25, _("to delay"));
                    o->labelsize(8);
                    o->argument(114);
                    o->callback((Fl_Callback *)callback);
                    Knob[i][o->argument()] = o;
                }
                o->end();
                tab[i] = o;
            }  // ==================================== end single tab
        }  // end of for
        {
            // ostringstream oss;
            // oss<<_("about");
            tablabel[i] = _("about");  // oss.str();
            Fl_Group *o = new Fl_Group(1, 10, 995, 515, tablabel[i].c_str());
            o->color((Fl_Color)246);
            o->labelsize(8);
            // o->argument(2);
            // int xtab=1;
            // o->labelcolor(FL_BACKGROUND2_COLOR);
            // o->callback((Fl_Callback*)tabcallback,&xtab);

            o->box(FL_BORDER_FRAME);
            // draw logo
            {
                Fl_Box *o = new Fl_Box(855, 450, 25, 25);
                o->image(image_miniMini);
            }
            {
                Fl_Help_View *o = new Fl_Help_View(200, 50, 600, 300, _("About Minicomputer"));
                o->box(FL_ROUNDED_BOX);
                o->labelsize(8);
                o->color((Fl_Color)246);
                // o->textcolor(FL_BACKGROUND2_COLOR);
                o->textfont(FL_HELVETICA_BOLD);
                o->labelcolor(FL_BACKGROUND2_COLOR);
                std::string Textausgabe;
                char version[] = _VERSION;
                Textausgabe = astrprintf(_("<html><body><i><center>version %s</center></i><br><p><br>a standalone industrial grade softwaresynthesizer for Linux<br><p><br>developed by Malte Steiner 2007-2009<p>distributed as free open source software under GPL3 licence<br><p>contact:<br><center>steiner@block4.com<br>http://www.block4.com<br>http://minicomputer.sourceforge.net</center></body></html>"),
                        version);
                o->value(Textausgabe.c_str());
            }
            o->end();
            tab[i] = o;
        }  // ==================================== end about tab

        o->end();
        tabs = o;
    }
    // ---------------------------------------------------------------- end of tabs
    /*{ Fl_Chart * o = new Fl_Chart(600, 300, 70, 70, _("eg"));
        o->bounds(0.0,1.0);
        o->type(Fl::LINE_CHART);
        o->insert(0, 0.5, NULL, 0);
        o->insert(1, 0.5, NULL, 0);
        o->insert(2, 1, NULL, 0);
        o->insert(3, 0.5, NULL, 0);
        EG[0]=o;

    }*/

    // ----------------------------------------- Multi
    {
        Fl_Input *o = new Fl_Input(10, 476, 106, 14, _("Multi"));
        o->box(FL_BORDER_BOX);
        // o->color(FL_FOREGROUND_COLOR);
        o->labelsize(8);
        o->textsize(8);
        o->align(FL_ALIGN_TOP_LEFT);
        // o->callback((Fl_Callback*)changemulti,NULL);
        // o->callback((Fl_Callback*)chooseMultiCallback,NULL); // for the roller
        o->tooltip(_("enter name for multisetup before storing it"));
        multichoice = o;
        Multichoice = o;
    }
    // roller for the multis:
    {
        Mw_Roller *o = new Mw_Roller(20, 492, 80, 10);
        o->type(FL_HORIZONTAL);
        o->tooltip(_("roll the list of multis, press load button for loading or "
                     "save button for storing"));
        o->minimum(0);
        o->maximum(127);
        o->step(1);
        o->box(FL_BORDER_FRAME);
        o->mouse_wheel_steps(128);
        o->callback((Fl_Callback *)multiRollerCallback, NULL);
        multiRoller = o;
    }
    {
        Fl_Value_Output *o = new Fl_Value_Output(180, 488, 20, 15, _("multiset"));
        o->box(FL_ROUNDED_BOX);
        o->color(FL_BLACK);
        o->labelsize(8);
        o->maximum(512);
        o->textsize(8);
        o->textcolor(FL_RED);
        multiDisplay = o;
    }
    {
        Fl_Button *o = new Fl_Button(206, 465, 59, 19, _("store multi"));
        o->tooltip(_("overwrite this multi"));
        o->box(FL_BORDER_BOX);
        o->labelsize(8);
        o->labelcolor((Fl_Color)1);
        o->callback((Fl_Callback *)storemulti, multichoice);
        sm = o;
    }
    {
        Fl_Button *o = new Fl_Button(126, 465, 70, 19, _("load multi"));
        o->tooltip(_("load current multi"));
        o->box(FL_BORDER_BOX);
        o->labelsize(8);
        o->labelcolor((Fl_Color)186);
        o->callback((Fl_Callback *)loadmulti, multichoice);
        lm = o;
    }
    // parameter tuning
    {
        Fl_Value_Input *o = new Fl_Value_Input(710, 470, 106, 14, _("current parameter"));
        // o->box(FL_BORDER_FRAME);
        o->box(FL_ROUNDED_BOX);
        // o->color(FL_FOREGROUND_COLOR);
        // o->selection_color(FL_FOREGROUND_COLOR);
        o->labelsize(8);
        o->textsize(8);
        o->range(-2, 2);
        // o->menubutton()->textsize(8);
        o->align(FL_ALIGN_TOP_LEFT);
        o->step(0.0001);
        o->callback((Fl_Callback *)finetune);
        paramon = o;
    }


    o->end();
    return o;
}

/**
 * constructor
 * calling straight the super class constructor
 */
Fenster::Fenster(int w, int h, const char *t)
    : Fl_Double_Window(w, h, t)
{
}
/**
 * constructor
 * calling straight the super class constructor
 */
Fenster::Fenster(int w, int h)
    : Fl_Double_Window(w, h)
{
}
/**
 * using the destructor to shutdown synthcore
 */
Fenster::~Fenster()
{

    printf("guittt");
    fflush(stdout);
    //sendClose(1);
    //~Fl_Double_Window();
}
/**
 * overload the eventhandler for some custom shortcuts
 * @param event
 * @return 1 when all is right
 */
int Fenster::handle(int event)
{
    switch (event) {
    case FL_KEYBOARD:
        if (tabs != NULL) {
            switch (Fl::event_key()) {
            case FL_F + 1:
                tabs->value(tab[0]);
                tabcallback(tabs, NULL);
                break;
            case FL_F + 2:
                tabs->value(tab[1]);
                tabcallback(tabs, NULL);
                break;
            case FL_F + 3:
                tabs->value(tab[2]);
                tabcallback(tabs, NULL);
                break;
            case FL_F + 4:
                tabs->value(tab[3]);
                tabcallback(tabs, NULL);
                break;
            case FL_F + 5:
                tabs->value(tab[4]);
                tabcallback(tabs, NULL);
                break;
            case FL_F + 6:
                tabs->value(tab[5]);
                tabcallback(tabs, NULL);
                break;
            case FL_F + 7:
                tabs->value(tab[6]);
                tabcallback(tabs, NULL);
                break;
            case FL_F + 8:
                tabs->value(tab[7]);
                tabcallback(tabs, NULL);
                break;
            }  // end of switch
        }  // end of if
        return 1;

    default:
        // pass other events to the base class...

        return Fl_Double_Window::handle(event);
    }
}
/*
void close_cb( Fl_Widget* o, void*) {
    printf("stop editor");
    fflush(stdout);
   exit(0);
}*/
