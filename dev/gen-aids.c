#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

#define TTLS 1500

#define DEBUG 0

FILE *titles;
FILE *concepts;

struct title
{
    char les[11];
    char unit[9];
    int interp;
    char *t;
};

struct title tlist[TTLS];
char line[200];

int main (int argc, char **argv)
{
    int i, j;
    char *p;
    int tcount;
    int concept;
    int title;
    
    if (argc < 2)
    {
        fprintf (stderr, "too few args\n");
        exit (1);
    }
    titles = fopen (argv[1], "r");
    if (titles == NULL)
    {
        fprintf (stderr, "opening titles (%s): %s\n", argv[1], strerror (errno));
        exit (1);
    }
    concepts = fopen (argv[2], "r");
    if (concepts == NULL)
    {
        fprintf (stderr, "opening concepts (%s): %s\n", argv[2], strerror (errno));
        exit (1);
    }
    for (i = 0; i < TTLS; i++)
    {
        fgets (line, 200, titles);
        if (feof (titles))
            break;
        if ((p = strchr (line, '\n')) == NULL)
        {
            fprintf (stderr, "line too long: %s\n", line);
            exit (1);
        }
        *p = '\0';
        tlist[i].les[10] = '\0';
        tlist[i].unit[8] = '\0';
        memcpy (tlist[i].les, line, 10);
        for (j = 9; j > 0; j--)
        {
            if (tlist[i].les[j] == ' ')
                tlist[i].les[j] = '\0';
            else
                break;
        }
        memcpy (tlist[i].unit, line + 12, 8);
        for (j = 7; j > 0; j--)
        {
            if (tlist[i].unit[j] == ' ')
                tlist[i].unit[j] = '\0';
            else
                break;
        }
        tlist[i].interp = (line[24] == 't');
        tlist[i].t = strdup (line + 36);
#if DEBUG
        printf ("%s %s %d %s\n", tlist[i].les, tlist[i].unit, tlist[i].interp, tlist[i].t);
#endif
    }
    tcount = i;
    concept = 0;
    title = -1;
    for (;;)
    {
        fgets (line, 200, concepts);
        if (feof (concepts))
            break;
        if ((p = strchr (line, '\n')) == NULL)
        {
            fprintf (stderr, "line too long: %s\n", line);
            exit (1);
        }
        *p = '\0';
        if (line[0] == 'c')
        {
            if (concept > 0)
            {
                if (title < 0)
                    printf ("* concept %d no match\n", concept);
                else
                    printf ("do      ent(%d,'7%s'7,'7%s'7,%d)\n",
                            concept, tlist[title].les,
                            tlist[title].unit, tlist[title].interp);
            }
            title = -1;
            concept++;
        }
#if DEBUG
        printf ("%d %s\n", concept, line);
#endif
        for (i = 0; i < tcount; i++)
        {
            if (strcmp (tlist[i].t, line + 8) == 0)
            {
                if (title < 0)
                    title = i;
                else if (i != title)
                    printf ("* ambiguous concept %d (%s/%s or %s/%s)\n",
                            concept, tlist[title].les, tlist[title].unit,
                            tlist[i].les, tlist[i].unit);
                break;
            }
        }
    }
    if (title < 0)
        printf ("* concept %d no match\n", concept);
    else
        printf ("do      ent(%d,'7%s'7,'7%s'7,%d)\n",
                concept, tlist[title].les,
                tlist[title].unit, tlist[title].interp);
}
