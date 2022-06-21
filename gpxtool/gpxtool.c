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

struct sPosList {
  struct sPosition sHead;
  struct sPosList *pNext;
};

struct sPosList *pListHead=NULL;

int fillNode(struct sPosition *pList, 
	     double dLatitude, double dLongitude, double dAltitude,
	     time_t tTimestamp, int nSat, double dHDOP)
{
  printf("Filling node at time %lld\n",tTimestamp);
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
  return fillNode(pList->pNext,dLatitude,dLongitude,dAltitude,tTimestamp, nSat, dHDOP);
}

int fillNodeList(struct sPosList *pList,
		double dLatitude, double dLongitude, double dAltitude,
		time_t tTimestamp, int nSat, double dHDOP)
{
  printf("Filling node list at time %lld\n",tTimestamp);
  pList->sHead.tTimestamp=tTimestamp;
  pList->sHead.pNext=malloc(sizeof(struct sPosition));
  fillNode(pList->sHead.pNext,dLatitude,dLongitude,dAltitude,tTimestamp, nSat, dHDOP);
  pList->pNext=NULL;
}

int addNodeList(struct sPosList *pList,
		double dLatitude, double dLongitude, double dAltitude,
		time_t tTimestamp, int nSat, double dHDOP)
{
  if(pList->sHead.tTimestamp==tTimestamp)
    {
      printf("Matched timestamp %lld\n",tTimestamp);
      return addNode(pList->sHead.pNext,dLatitude,dLongitude,dAltitude,tTimestamp, nSat, dHDOP);
    }
  if(pList->pNext)
    {
      if(pList->pNext->sHead.tTimestamp<=tTimestamp)
	return addNodeList(pList->pNext,dLatitude,dLongitude,dAltitude,tTimestamp, nSat, dHDOP);
      printf("Adding timestamp %lld between %lld and %lld\n",tTimestamp,pList->sHead.tTimestamp,pList->pNext->sHead.tTimestamp);
      struct sPosList *pTemp=pList->pNext;
      pList->pNext=malloc(sizeof(struct sPosList));
      fillNodeList(pList->pNext,dLatitude,dLongitude,dAltitude,tTimestamp, nSat, dHDOP);
      pList->pNext->pNext=pTemp;
    }
  else
    {
      pList->pNext=malloc(sizeof(struct sPosList));
      fillNodeList(pList->pNext,dLatitude,dLongitude,dAltitude,tTimestamp, nSat, dHDOP);
    }
}

int procNodeList(struct sPosList *pList)
{
  int n2DCount=0, n3DCount=0;
  struct sPosition *pPos=pList->sHead.pNext;
  pList->sHead.dLatitude=0;
  pList->sHead.dLongitude=0;
  pList->sHead.dAltitude=0;
  printf("Time %lld\n",pList->sHead.tTimestamp);
  while(pPos)
    {
      pList->sHead.dLatitude+=pPos->dLatitude;
      pList->sHead.dLongitude+=pPos->dLongitude;
      printf("Point %d Lat %f Lon %f\n",n2DCount,pPos->dLatitude,pPos->dLongitude);
      n2DCount++;
      if(pPos->dAltitude)
	{
	  pList->sHead.dAltitude+=pPos->dAltitude;
	  n3DCount++;
	}
      pPos=pPos->pNext;   
    }
  pList->sHead.dLatitude/=n2DCount;
  pList->sHead.dLongitude/=n2DCount;
  if(n3DCount)
    pList->sHead.dAltitude/=n3DCount;
  if(pList->pNext)
    procNodeList(pList->pNext);
  return 0;
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
    {
      if(epoch<pListHead->sHead.tTimestamp)
	{
	  printf("Earlier timestamp tthan %lld at %lld\n",pListHead->sHead.tTimestamp,epoch);
	  struct sPosList *pList=malloc(sizeof(struct sPosList));
	  fillNodeList(pList,dlat,dlon,dele,epoch,nsat,dhdop);
	  pList->pNext=pListHead;
	  pListHead=pList;
	}
      else
	addNodeList(pListHead,dlat,dlon,dele,epoch,nsat,dhdop);
    }
  else
    {
      pListHead=malloc(sizeof(struct sPosList));
      fillNodeList(pListHead,dlat,dlon,dele,epoch,nsat,dhdop);
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
  for(i=1;i<argc;i++)
    {
      printf("Parsing %s\n",argv[i]);
      procFile(argv[i]);
    }

  xmlCleanupParser();

  procNodeList(pListHead);

  while(pListHead)
    {
      struct sPosList *pNext=pListHead->pNext;
      struct sPosition *pList=pListHead->sHead.pNext;
      while(pList)
	{
	  struct sPosition *pTemp;
	  printf("Latitude %f, Longitude %f, Altitude %f, Time %lld, Sat %d, HDOP %f\n",
		 pList->dLatitude,pList->dLongitude,pList->dAltitude,
		 pList->tTimestamp,pList->nSat,pList->dHDOP);
	  pTemp=pList;
	  pList=pList->pNext;
	  free(pTemp);
	}
      printf("Latitude %f, Longitude %f, Altitude %f, Time %lld\n",
	     pListHead->sHead.dLatitude,pListHead->sHead.dLongitude,pListHead->sHead.dAltitude,
	     pListHead->sHead.tTimestamp);
      free(pListHead);
      pListHead=pNext;
    }
  
  return 0;
}
