//
//  cxUtil.m
//  cxEngine
//
//  Created by xuhua on 9/24/13.
//  Copyright (c) 2013 xuhua. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <AudioToolbox/AudioToolbox.h>
#import <AudioToolbox/ExtendedAudioFile.h>
#include <core/cxUtil.h>
#include <core/cxPlayer.h>
#include <core/cxOpenGL.h>

cxString cxCreateTXTTextureData(cxConstChars txt,cxConstChars font,cxTextAttr attr)
{
    CX_RETURN(txt == NULL, NULL);
    NSString *text = [NSString stringWithUTF8String:txt];
    UIFont *pfont = nil;
    if(font == NULL){
        pfont = attr.bold?[UIFont boldSystemFontOfSize:attr.size]:[UIFont systemFontOfSize:attr.size];
    }else{
        pfont = [UIFont fontWithName:[NSString stringWithUTF8String:font] size:attr.size];
    }
    if(pfont == nil){
        pfont = attr.bold?[UIFont boldSystemFontOfSize:attr.size]:[UIFont systemFontOfSize:attr.size];
    }
    NSDictionary * attributes = [NSDictionary dictionaryWithObject:pfont forKey:NSFontAttributeName];
    CGSize size = [text sizeWithAttributes:attributes];
    cxInt width = (cxInt)floorf(size.width);
    cxInt height = (cxInt)floorf(size.height);
    cxInt bufsiz = width * height * 4 + sizeof(cxSize2i);
    cxPointer buffer = allocator->malloc(bufsiz);
    cxPointer data = buffer + sizeof(cxSize2i);
    memset(buffer, 0, bufsiz);
    CGColorSpaceRef color = CGColorSpaceCreateDeviceRGB();
    CGContextRef context = CGBitmapContextCreate(data,width,height,8,width*4,color,kCGImageAlphaPremultipliedLast|kCGBitmapByteOrder32Big);
    CGColorSpaceRelease(color);
    CGContextSetRGBFillColor(context, 1, 1, 1, 1);
    CGContextTranslateCTM(context, 0.0f, height);
    CGContextScaleCTM(context, 1.0f, -1.0f);
    UIGraphicsPushContext(context);
    [text drawAtPoint:CGPointMake(0, 0) withAttributes:attributes];
    UIGraphicsPopContext();
    CGContextRelease(context);
    ((cxSize2i *)buffer)->w = width;
    ((cxSize2i *)buffer)->h = height;
    return cxStringAttach(buffer, bufsiz);
}

cxString cxWAVSamplesWithFile(cxConstChars file,cxUInt *format,cxUInt *freq)
{
    cxString cxPath = cxAssetsPath(file);
    AudioStreamBasicDescription fileformat;
    UInt32 formatsize = sizeof(fileformat);
    NSString *path = [NSString stringWithUTF8String:cxStringBody(cxPath)];
    CFURLRef fileURL = (__bridge CFURLRef)[NSURL fileURLWithPath:path];
    UInt64 fileDataSize = 0;
    UInt32 thePropertySize = sizeof(UInt64);
    AudioFileID afid = 0;
    void *theData = NULL;
    if(AudioFileOpenURL(fileURL, kAudioFileReadPermission, 0, &afid)){
        goto completed;
    }
    if(AudioFileGetProperty(afid, kAudioFilePropertyDataFormat, &formatsize, &fileformat)){
        CX_ERROR("get format error");
        goto completed;
    }
    if(!AudioFileGetProperty(afid, kAudioFilePropertyAudioDataByteCount, &thePropertySize, &fileDataSize)){
        theData = allocator->calloc(1,(cxInt)fileDataSize);
        AudioFileReadBytes(afid, false, 0, (UInt32 *)&fileDataSize, theData);
        *freq = (ALsizei)fileformat.mSampleRate;
        if(fileformat.mBitsPerChannel == 16){
            *format = (fileformat.mChannelsPerFrame > 1) ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
        }else{
            *format = (fileformat.mChannelsPerFrame > 1) ? AL_FORMAT_STEREO8 : AL_FORMAT_MONO8;
        }
    }
completed:
    AudioFileClose(afid);
    if(theData == NULL){
        return NULL;
    }
    return cxStringAttach(theData, (UInt32)fileDataSize);
}

void cxUtilPrint(cxConstChars type,cxConstChars file,int line,cxConstChars format,va_list ap)
{
    char buffer[CX_MAX_LOGGER_LENGTH];
    vsnprintf(buffer, CX_MAX_LOGGER_LENGTH, format, ap);
    NSLog(@"[%s:%d] %s:%s\n",file,line,type,buffer);
}

cxString cxAssetsPath(cxConstChars file)
{
    NSString *path = [[NSBundle mainBundle] resourcePath];
    if(file != NULL){
        return cxStringCreate("%s/%s",[path UTF8String],file);
    }else{
        return cxStringCreate("%s",[path UTF8String]);
    }
}

cxString cxDocumentPath(cxConstChars file)
{
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *path = [paths objectAtIndex:0];
    if(file != NULL){
        return cxStringCreate("%s/%s",[path UTF8String],file);
    }else{
        return cxStringConstChars([path UTF8String]);
    }
}

cxString cxLocaleLang()
{
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSArray *languages = [defaults objectForKey:@"AppleLanguages"];
    NSString *current = [languages objectAtIndex:0];
    NSDictionary* temp = [NSLocale componentsFromLocaleIdentifier:current];
    NSString * lngcode = [temp objectForKey:NSLocaleLanguageCode];
    return cxStringCreate("%s",[lngcode UTF8String]);
}
