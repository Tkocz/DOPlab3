/*
* File: cmdftn.c
* ------------
* This file implements the cmdftn.h interface.
*/
#include "cmdfnt.h"
#include "scanadt.h"

/* static variables */
static symtabADT commandTable;

/* Exported entries */

void DefineCommand(string cmd, commandFnT fn)
{
	commandEntryT entry;
	entry = New(commandEntryT);
	entry->fn = fn;
	Enter(commandTable, cmd, entry);
}
void ExecuteCommand(string cmd, environmentADT env)
{
	commandEntryT entry;
	entry = Lookup(commandTable, cmd);
	if (entry == UNDEFINED) {
		printf("Undefined command: %s\n Seek :help!\n", cmd);
		return;
	}
	entry->fn(env);
}
void InitCommandTable(void)
{
	commandTable = NewSymbolTable();
}
