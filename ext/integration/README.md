ObjMaster Ext / Integration
===========================

Non-c++ integration facade. Can be used to integrate with C, C#/Mono/.NET or
eve maybe with java and other languages. The code here is tested as a DLL that
is being used as an integration point for Unity3D use cases, but should work
in a variety of situations.

Using this facade is architecturally not as optimal as direct c++ usage, but is
good-enough for most cases. Also, when integrating with a third party engine,
inner technical details for texture loading, and vertex structures are handled
by that engine anyways.

Designed especially for Unity3D originally.
