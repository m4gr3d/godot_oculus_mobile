package org.godotengine.plugin.vr.oculus.mobile.api

import android.view.Surface
import org.godotengine.plugin.vr.oculus.mobile.OvrMobilePlugin
import java.util.concurrent.atomic.AtomicLong

/**
 * Provides APIs to access and render on a compositor layer.
 */
class OvrLayer constructor(val ovrPlugin: OvrMobilePlugin, val width: Int, val height: Int) {

    // Keep in sync with ovr_layer.h
    enum class VideoScreenFormat {
        EQR_MONO_180,
        EQR_MONO_360,
        EQR_LEFT_RIGHT_180,
        EQR_TOP_BOTTOM_360,
        RECTILINEAR_20X9_MONO,
    };

    companion object {
        const val RELEASED_LAYER_POINTER = 0L
    }

    interface SurfaceListener {
        fun onSurfaceCreated(surface: Surface)

        fun onSurfaceDestroyed()
    }

    private val surfaceListeners: MutableSet<SurfaceListener> = mutableSetOf()
    private val nativeLayerPointer: AtomicLong = AtomicLong(RELEASED_LAYER_POINTER)

    init {
        ovrPlugin.runOnRenderThread {
            nativeLayerPointer.set(nativeGenerateLayer(width, height))
            if (nativeLayerPointer.get() != RELEASED_LAYER_POINTER) {
                notifySurfaceCreated(getSurface()!!)
            }
        }
    }

    private fun notifySurfaceCreated(surface: Surface) {
        ovrPlugin.runOnUiThread {
            for (listener in surfaceListeners) {
                listener.onSurfaceCreated(surface)
            }
        }
    }

    private fun notifySurfaceDestroyed() {
        ovrPlugin.runOnUiThread {
            for (listener in surfaceListeners) {
                listener.onSurfaceDestroyed()
            }
        }
    }

    fun addSurfaceListener(listener: SurfaceListener) {
        surfaceListeners.add(listener)
        if (!isReleased()) {
            listener.onSurfaceCreated(getSurface()!!)
        }
    }

    fun removeSurfaceListener(listener: SurfaceListener) {
        surfaceListeners.remove(listener)
    }

    private fun isReleased() = nativeLayerPointer.get() == RELEASED_LAYER_POINTER

    fun release() {
        if (!isReleased()) {
            ovrPlugin.runOnRenderThread {
                nativeDestroyLayer(nativeLayerPointer.get())
                nativeLayerPointer.set(RELEASED_LAYER_POINTER)
                notifySurfaceDestroyed()
            }
        }
    }

    fun getSurface(): Surface? {
        if (isReleased()) {
            return null
        }

        return nativeGetLayerSurface(nativeLayerPointer.get())
    }

    fun setFormat(format: VideoScreenFormat) {
        if (!isReleased()) {
            ovrPlugin.runOnRenderThread {
                nativeSetFormat(nativeLayerPointer.get(), format.ordinal);
            }
        }
    }

    fun setSurfaceProtected(isProtected: Boolean) {
        if (!isReleased()) {
            ovrPlugin.runOnRenderThread {
                nativeSetSurfaceProtected(nativeLayerPointer.get(), isProtected);
            }
        }
    }

    private external fun nativeGenerateLayer(width: Int, height: Int): Long

    private external fun nativeSetFormat(layerPointer: Long, format: Int)

    private external fun nativeSetSurfaceProtected(layerPointer: Long, isProtected: Boolean)

    private external fun nativeDestroyLayer(layerPointer: Long)

    private external fun nativeGetLayerSurface(layerPointer: Long): Surface?
}
