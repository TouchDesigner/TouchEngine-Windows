TouchEngine
===========

TouchEngine provides an API to load and render TouchDesigner components.


Instances And TouchDesigner Installations
-----------------------------------------

TouchEngine requires users have an installed version of TouchDesigner to work with loaded components. TouchEngine will locate an installed version suitable for use on the user's system.

Users can specify a particular version to use by including a folder named "TouchEngine" alongside the component .tox being loaded. This folder is a renamed TouchDesigner installation directory (on Windows) or application (on macOS), or a file-system link to an installation or application location (either a symbolic link or a Windows Explorer shortcut or macOS Finder alias).


TEObjects
---------

Objects created or returned from the TouchEngine API are reference-counted, and you take ownership of objects from the API. If you use an API function with "Create" or "Get" in its name which returns a TEObject (including via a function argument), you must use `TERelease()` when you are finished with the object.

    TELinkInfo *info;
    TEResult result = TEInstanceLinkGetInfo(instance, identifier, &info);
    if (result == TEResultSuccess)
    {
        // You become the owner of the TELinkInfo object
        // use the object...
        // ...
        // ...and then release it
        TERelease(&info);
        // (info is set to NULL by TERelease()) 
    }

You can use `TERetain()` to increase the reference-count of an object.

Some functions accept or return several types of TEObject. Use `TEGetType()` to check the type of a TEObject returned from such functions, then cast the value to the actual type.


Links
-----

Individual inputs and outputs of an instance are referred to as *links*. In TouchDesigner terms, links combine parameters and In and Out operators.


Creating and Configuring Instances
----------------------------------

An instance requires two callbacks: one for instance events, and one to receive link events:

    void eventCallback(TEInstance * instance, TEEvent event, TEResult result, int64_t start_time_value, int32_t start_time_scale, int64_t end_time_value, int32_t end_time_scale, void * info)
    {
        // handle the event
    }
    
    void linkCallback(TEInstance * instance, TELinkEvent event, const char *identifier, void * info)
    {
        // handle the link event
    }

A single instance can be re-used to load several components. Only one component can be loaded in an instance at a time (but any number of instances can co-exist). Maximise performance by re-using an existing instance rather than creating a new one where possible.

Create an instance:

    TEInstance *instance;
    TEResult result = TEInstanceCreate(eventCallback, linkCallback, NULL, &instance);
    if (result == TEResultSuccess)
    {
        // Continue to use the instance
    }

If working with textures, create and associate a TEGraphicsContext suitable for your needs. Alternatively create and associate a TEAdapter so the instance does its work on the intended device. If neither are associated, the instance will select a device as it sees fit.

    // See TEGraphicsContext.h to create a suitable context
    if (result == TEResultSuccess)
    {
        result = TEInstanceAssociateGraphicsContext(instance, context);
    }

You may wish to set a frame-rate to match your intended render rate:

    if (result == TEResultSuccess)
    {
        // for example, this would set 30 FPS
        result = TEInstanceSetFrameRate(instance, 30, 1);
    }

Load a component:

    if (result == TEResultSuccess)
    {
        result = TEInstanceLoad(instance, "sample.tox", TETimeExternal);
    }

Loading begins immediately.

During loading you may receive link callbacks with the event TELinkEventAdded for any links on the instance.

An instance is loaded suspended. Once configured, resuming the instance will permit rendering (and start playback in TETimeInternal mode):

    if (result == TEResultSuccess)
    {
        result = TEInstanceResume(instance);
    }


Once loading has completed you will receive an event callback with the event TEEventInstanceDidLoad, and a TEResult indicating success or any warning or error.


Rendering An Instance
---------------------

Rendering is performed asynchronously according to the TETimeMode of the instance.

For a TETimeExternal instance, rendering is driven by your API calls. Times passed to `TEInstanceStartFrameAtTime()` determine progress.

For a TETimeInternal instance, rendering continues in the background at the instance's frame-rate. Output is driven by calls to `TEInstanceStartFrameAtTime()`.

For both modes, after starting a frame the instance's link callback will be invoked for outputs whose value has changed. The completion of a frame you have requested is marked by the event callback receiving TEEventFrameDidFinish with a TEResult indicating success or any warning or error.


Working With TELinkTypeFloatBuffer
----------------------------------

Float buffer links take or emit a buffer of float values arranged in channels. They can contain time-based or static values. One example of time-based values is audio data. An example of static values might be coordinates, perhaps with a channel for each dimension.

To allow the most efficient memory re-use inside TouchEngine, for each input link create a TEFloatBuffer once (using `TEFloatBufferCreate()` or `TEFloatBufferCreateTimeDependent()`) and then create subsequent buffers from the original buffer using `TEFloatBufferCreateCopy()`.

Time-dependent buffers can be added to the instance with `TEInstanceLinkAddFloatBuffer()`, which adds the buffer to an internal queue.

Calling `TEInstanceLinkSetFloatBufferValue()` replaces any current value as well as clearing any time-dependent values previously queued.

To receive time-dependent buffers from the instance, call `TEInstanceLinkGetFloatBufferValue()` from your `TEInstanceLinkCallback`. No further buffers will be received during the callback, allowing you to safely dequeue them without risk of loss.


Working with TELinkTypeStringData
--------------------------------------

String data links can be tables or a single string value.

When working with table inputs, to allow the most efficient memory re-use inside TouchEngine, for each link create a TETable once using `TETableCreate()` and then create subsequent tables from the original table using `TETableCreateCopy()`.

Set a single string value on a string data input with `TEInstanceLinkSetStringValue()`, or set a table value with `TEInstanceLinkSetTableValue()`. To receive string data values from an output, use `TEInstanceLinkGetObjectValue()` and then use `TEGetType()` on the returned value to determine if it is a TEString or TETable.


Working with TELinkTypeTexture
-----------------------------------

Using texture inputs and outputs on an instance in most cases requires the use of a TEGraphicsContext of a suitable type for the texture. For example, to set a TED3D11Texture as an input link requires a TED3D11Context for the instance to do any necessary work in.

One-time setup:

    TED3D11Context *context;
    TEResult result = TED3D11ContextCreate(device, &context);
    if (result != TEResultSuccess)
    {
        // deal with the error
    }

Setting an input:

    TED3D11Texture *texture = TED3D11TextureCreate(tex, false);
    TEResult result = TEInstanceLinkSetTextureValue(instance, identifier, texture, context);
    // Release the texture - the instance will have retained it if necessary
    TERelease(&texture);

Getting an output:

    TETexture *value;
    TEResult result = TEInstanceLinkGetTextureValue(instance, identifier, TELinkValueCurrent, &value);
    if (result == TEResultSuccess &&
        value != NULL &&
        TETextureGetType(value) == TETextureTypeDXGI)
    {
        TED3D11Texture *texture;
        result = TED3D11ContextCreateTexture(context, value, &texture);
        if (result == TEResultSuccess)
        {
            // Use the instantiated texture here
            // ...
            TERelease(&texture);
        }
    }
    TERelease(&value);


Allowing users to reference known TouchDesigner objects
-------------------------------------------------------

In some cases your users may want to refer to inputs and outputs by the name they have used for entities inside a loaded TouchDesigner component. Combined use of the `name` and `domain` members of the `TELinkInfo` struct allow for a one-to-one reference to entities within a component. For example, a user may wish to refer to a TouchDesigner operator with the name "out1" to locate an output. To match that, locate a link with the domain `TELinkDomainOperator` and name "out1". Note that `name` is only a reliable identifier within a single domain - the same name can occur in multiple domains.

If you need to use domains in UI, the two domains which users might expect to be able to refer to, and textual names and abbreviations which will be familiar to them, are:

|Domain                |Name      |Abbreviation |
|----------------------|----------|-------------|
|TELinkDomainParameter |Parameter |par          |
|TELinkDomainOperator  |Operator  |op           |
