#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <libxml/tree.h>

#define __USE_XOPEN  // For strptime
#include <time.h>

struct sPosition {
  double dLatitude;
  double dLongitude;
  double dAltitude;
  time_t tTimestamp;
  int nSat;
  double dHDOP;
  struct sPosition *pNext;
};

struct sPosition *pListHead=NULL;

int fillNode(struct sPosition *pList, 
	     double dLatitude, double dLongitude, double dAltitude,
	     time_t tTimestamp, int nSat, double dHDOP)
{
  pList->dLatitude=dLatitude;
  pList->dLongitude=dLongitude;
  pList->dAltitude=dAltitude;
  pList->tTimestamp=tTimestamp;
  pList->nSat=nSat;
  pList->dHDOP=dHDOP;
  pList->pNext=NULL;
}

int addNode(struct sPosition *pList,
	    double dLatitude, double dLongitude, double dAltitude,
	    time_t tTimestamp, int nSat, double dHDOP)
{
  if(pList->pNext)
    return addNode(pList->pNext,dLatitude,dLongitude,dAltitude,tTimestamp, nSat, dHDOP);
  
  pList->pNext=malloc(sizeof(struct sPosition));
  fillNode(pList->pNext,dLatitude,dLongitude,dAltitude,tTimestamp, nSat, dHDOP);
}

void procPoint(xmlDocPtr doc, xmlNodePtr segchild)
{
  char *end = 0;
  struct tm ttime;
  time_t epoch;
  double dlat,dlon,dele,dhdop;
  int nsat;
  xmlChar *lat = xmlGetProp(segchild, (xmlChar*)"lat");
  xmlChar *lon = xmlGetProp(segchild, (xmlChar*)"lon");
  printf("Lat %s lon %s\n",(char *)lat, (char *)lon);
  dlat=strtod((char*)lat,&end);
  dlon=strtod((char*)lon,&end);
  xmlNodePtr pointchild=segchild->xmlChildrenNode;
  while(pointchild)
    {
      printf("point child node: %s\n",(char *)pointchild->name);
      if(!strcmp((char *)pointchild->name,"ele"))
	{					
	  xmlChar *ele = xmlNodeListGetString(doc, pointchild->xmlChildrenNode, 1);
	  printf("Ele %s\n",(char *)ele);
	  dele=strtod((char*)ele,&end);
	}
      else if(!strcmp((char *)pointchild->name,"time"))
	{	
	  xmlChar *time = xmlNodeListGetString(doc, pointchild->xmlChildrenNode, 1);
	  printf("Time %s\n",(char *)time);
	  strptime((char*)time, "%Y-%m-%dT%H:%M:%SZ", &(ttime));
	  epoch = mktime(&ttime);
	  printf("Time %d\n",epoch);
	}
      else if(!strcmp((char *)pointchild->name,"sat"))
	{					
	  xmlChar *sat = xmlNodeListGetString(doc, pointchild->xmlChildrenNode, 1);
	  printf("Sat %s\n",(char *)sat);
	  nsat=strtol((char*)sat,&end,10);
	}
      if(!strcmp((char *)pointchild->name,"hdop"))
	{					
	  xmlChar *hdop = xmlNodeListGetString(doc, pointchild->xmlChildrenNode, 1);
	  printf("HDOP %s\n",(char *)hdop);
	  dhdop=strtod((char*)hdop,&end);
	}
      pointchild=pointchild->next;      
    } 
  if(pListHead)
    addNode(pListHead,dlat,dlon,dele,epoch,nsat,dhdop);
  else
    {
      pListHead=malloc(sizeof(struct sPosition));
      fillNode(pListHead,dlat,dlon,dele,epoch,nsat,dhdop);
    }
}

void procFile(char *szName)
{

  // Load file
  xmlDocPtr doc = xmlParseFile(szName);
 
  if(doc)
    {
      xmlNodePtr cur = xmlDocGetRootElement(doc);
      printf("root node: %s\n",(char *)cur->name);
      if(strcmp((char *)cur->name,"gpx"))
	{
	  printf("Not a GPX file\n");
	}
      else
	{
	  xmlNodePtr child=cur->xmlChildrenNode;
	  while(child)
	    {
	      printf("child node: %s\n",(char *)child->name);
	      if(!strcmp((char *)child->name,"trk"))
		{
		  xmlNodePtr trackchild=child->xmlChildrenNode;
		  while(trackchild)
		    {
		      printf("track child node: %s\n",(char *)trackchild->name);
		      if(!strcmp((char *)child->name,"trk"))
			{
			  xmlNodePtr segchild=trackchild->xmlChildrenNode;
			  while(segchild)
			    {
			      printf("seg child node: %s\n",(char *)segchild->name);
			      if(!strcmp((char *)segchild->name,"trkpt"))
				procPoint(doc,segchild);
			      segchild=segchild->next;
			    }
			}
		      trackchild=trackchild->next;
		    }
		}
	      child=child->next;
	    }
	}
      xmlFreeDoc(doc);
    }
}

int main(int argc, char *argv[])
{
  int i,j;

  if (argc<2) 
    {
      printf("No file name given!\n");
      return 1;
    }
  
  // Initialize libxml
  xmlInitParser();
  
  // Load file
  procFile(argv[1]);

  xmlCleanupParser();

  while(pListHead)
    {
      struct sPosition *pNext=pListHead->pNext;
      printf("Latitude %f, Longitude %f, Altitude %f, Time %lld, Sat %d, HDOP %f\n",
	     pListHead->dLatitude,pListHead->dLongitude,pListHead->dAltitude,
	     pListHead->tTimestamp,pListHead->nSat,pListHead->dHDOP);
      free(pListHead);
      pListHead=pNext;
    }
  
  return 0;
}
