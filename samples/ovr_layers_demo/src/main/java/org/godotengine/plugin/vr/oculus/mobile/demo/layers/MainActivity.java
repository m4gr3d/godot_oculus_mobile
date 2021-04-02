package org.godotengine.plugin.vr.oculus.mobile.demo.layers;

import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.Surface;

import com.google.android.exoplayer2.C;
import com.google.android.exoplayer2.DefaultRenderersFactory;
import com.google.android.exoplayer2.ExoPlayerFactory;
import com.google.android.exoplayer2.RenderersFactory;
import com.google.android.exoplayer2.SimpleExoPlayer;
import com.google.android.exoplayer2.drm.DrmSessionManager;
import com.google.android.exoplayer2.drm.ExoMediaDrm;
import com.google.android.exoplayer2.drm.FrameworkMediaCrypto;
import com.google.android.exoplayer2.source.ExtractorMediaSource;
import com.google.android.exoplayer2.source.MediaSource;
import com.google.android.exoplayer2.source.dash.DashMediaSource;
import com.google.android.exoplayer2.source.dash.DefaultDashChunkSource;
import com.google.android.exoplayer2.source.dash.manifest.DashManifestParser;
import com.google.android.exoplayer2.trackselection.DefaultTrackSelector;
import com.google.android.exoplayer2.upstream.DataSource;
import com.google.android.exoplayer2.upstream.DefaultDataSourceFactory;
import com.google.android.exoplayer2.upstream.DefaultHttpDataSourceFactory;
import com.google.android.exoplayer2.upstream.RawResourceDataSource;
import com.google.android.exoplayer2.util.Util;

import org.godotengine.godot.Godot;
import org.godotengine.godot.plugin.GodotPlugin;
import org.godotengine.godot.plugin.GodotPluginRegistry;
import org.godotengine.godot.xr.XRMode;
import org.godotengine.plugin.vr.oculus.mobile.OvrMobilePlugin;
import org.godotengine.plugin.vr.oculus.mobile.api.OvrLayer;
import org.jetbrains.annotations.NotNull;

import java.util.Arrays;

public class MainActivity extends Godot {

    private static final String TAG = "GodotApp";

    private SimpleExoPlayer exoPlayer = null;
    /* @Nullable */ private ExoMediaDrm<FrameworkMediaCrypto> mediaDrm = null;
    private OvrLayer ovrLayer;

    @Override
    public void onCreate(Bundle bundle) {
        super.onCreate(bundle);
    }

    private MediaSource buildMediaSource(Uri uri, DataSource.Factory factory) {
        @C.ContentType int type = Util.inferContentType(uri);
        switch (type) {
            case C.TYPE_DASH:
                return new DashMediaSource.Factory(
                        new DefaultDashChunkSource.Factory(factory), factory)
                        .setManifestParser(new DashManifestParser())
                        .createMediaSource(uri);
            case C.TYPE_OTHER:
                return new ExtractorMediaSource.Factory(factory).createMediaSource(uri);
            default: {
                throw new IllegalStateException("Unsupported media source type for " + uri);
            }
        }
    }

    @Override
    protected void onGodotMainLoopStarted() {
        super.onGodotMainLoopStarted();

        GodotPlugin ovrPlugin = GodotPluginRegistry.getPluginRegistry().getPlugin("OVRMobile");
        if (!(ovrPlugin instanceof OvrMobilePlugin)) {
            throw new IllegalStateException("Unable to retrieve OVRMobile plugin");
        }

        Log.e(TAG, "CREATING SURFACE FROM JAVA");
        ovrLayer = new OvrLayer((OvrMobilePlugin) ovrPlugin, 1280, 720);
        Log.e(TAG, "CREATED SURFACE FROM JAVA");
        ovrLayer.addSurfaceListener(new OvrLayer.SurfaceListener() {
            @Override
            public void onSurfaceCreated(@NotNull Surface surface) {
                Log.i(TAG, "Starting media playback.");
                try {
                    // ovrLayer.setFormat(VideoScreenFormat.RECTILINEAR_20X9_MONO);
                    DefaultHttpDataSourceFactory httpDataSourceFactory =
                            new DefaultHttpDataSourceFactory(
                                    Util.getUserAgent(getApplicationContext(), TAG));
                    DataSource.Factory dataSourceFactory =
                            new DefaultDataSourceFactory(
                                    getApplicationContext(),
                                    Util.getUserAgent(getApplicationContext(), TAG));

                    // START Set up DRM.
                    DrmSessionManager<FrameworkMediaCrypto> drmSessionManager = null;
                    // String licenseUrl = "";
                    // mediaDrm = FrameworkMediaDrm.newInstance(C.WIDEVINE_UUID);
                    // HttpMediaDrmCallback drmCallback =
                    //         new HttpMediaDrmCallback(licenseUrl, httpDataSourceFactory);
                    // drmSessionManager =
                    //         new DefaultDrmSessionManager<FrameworkMediaCrypto>(
                    //                 C.WIDEVINE_UUID, mediaDrm, drmCallback, null, null, null);
                    // ovrLayer.setSurfaceProtected(true);
                    // END Set up DRM.

                    DefaultTrackSelector trackSelector = new DefaultTrackSelector();
                    trackSelector.setParameters(
                            trackSelector.getParameters().buildUpon()
                                    .setMaxVideoSize(3840, Integer.MAX_VALUE)
                                    .build());

                    RenderersFactory renderersFactory =
                            new DefaultRenderersFactory(getApplicationContext(), drmSessionManager);
                    exoPlayer = ExoPlayerFactory.newSimpleInstance(renderersFactory, trackSelector);
                    Log.e(TAG, "GETTING SURFACE FROM JAVA");
                    exoPlayer.setVideoSurface(ovrLayer.getSurface());
                    Log.e(TAG, "GOT SURFACE FROM JAVA");
                    // Uri playUri = Uri.parse("");
                    Uri playUri =
                            RawResourceDataSource.buildRawResourceUri(R.raw.checker_video_16x9);
                    MediaSource mediaSource = buildMediaSource(playUri, dataSourceFactory);
                    exoPlayer.setPlayWhenReady(true);
                    exoPlayer.prepare(mediaSource);
                    exoPlayer.setVolume(1.0f);
                } catch (Exception e) {
                    Log.e(TAG, "Exception starting video: ", e);
                }
            }

            @Override
            public void onSurfaceDestroyed() {
                if (exoPlayer != null) {
                    exoPlayer.release();
                }
            }
        });
    }

    @Override
    public void onStop() {
        super.onStop();
        if (exoPlayer != null) {
            exoPlayer.release();
        }
        if (ovrLayer != null) {
            ovrLayer.release();
        }
        finish();
    }

    @Override
    protected String[] getCommandLine() {
        String[] original = super.getCommandLine();
        String[] updated = Arrays.copyOf(original, original.length + 1);
        updated[original.length] = XRMode.OVR.cmdLineArg;
        return updated;
    }
}
