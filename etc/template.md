# Page Template #
This page can be used as a starting point for writing wiki pages, README.md and other documentation for omegalib or omeglib modules

## Versioning ##
> :heavy_check_mark: **VERSION** 14.0

The version quote is used at the beginning of a pae or section to indicate the content is correct from that version of the omegalib core or module and above.

## API ##
Major sections on the page are always level 2 headers

### foo ###
Here we have a general description of class foo. It is fine to have sections (ie for global functions) that have no description.
Classes, function groups and any grouping of reference items is a level 3 header

#### doFoo ####
> string doFoo(int x, [SceneNode] y)

The first line after a reference entry is a reference quote section describing the usage or syntax of this entry. 
After that, we have an optional description of the entry

> :memo: **NOTE** we can also include note sections to point out something interesting about this entry

> :memo: **NOTE** it is fine to have multiple notes, but separate them with an empty line or they will 
> end up on the same line

Sometimes we want to give an example of how to use this entry, that's also done with a quote section:
> :mortar_board: **EXAMPLE** (following the example title we can have an optional description)
```python
# Send all events from this server to a specific mission control client
def onEvent()
  e = getEvent()
  server = getMissionControlServer()
  target = server.findConnection('targetClient')
  if(target != None):
    server.sendEventTo(e, target)

> setEventFunction(onEvent) # If we have an empty line in our code we need to add the > again
```

> :mortar_board: **EXAMPLE** the example description is particularly useful when we want to have multiple 
> examples side by side
```c
include <stdio.h>
int main()
{
  printf("hello world\n");
  return 0;
}
```

If we want to point users to somewhere else interesting we can add a see also quote:
> :eyes: **SEE ALSO** [EngineModule::requestOpenGLProfile](http://uic-evl.github.io/omegalib/reference/html/classomega_1_1_engine_module.html#a30b94f677ef951a13a2a2868ac3f3f25)


[SceneNode]: https://github.com/uic-evl/omegalib/wiki/SceneNode
