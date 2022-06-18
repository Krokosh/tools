#include <stdio.h>

#include <libxml/tree.h>

#define __USE_XOPEN  // For strptime
#include <time.h>

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
  xmlDocPtr doc = xmlParseFile(argv[1]);
 
  char *end = 0;
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
				{
				  struct tm ttime;
				  time_t epoch;
				  double dlat,dlon,dele;
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
				      pointchild=pointchild->next;
					
				    }
				}
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
  xmlCleanupParser();
  
  return 0;
}
