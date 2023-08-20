#include "check.h"
#include "nicegraf-wrappers.h"
#include "nicegraf.h"
#include "shader-loader.h"

#include <stdio.h>

void ngf_test_draw(ngf::image& output_image, ngf_frame_token frame_token) {
  // Initialize the triangle test
  ngf::render_target     offscreen_rt;
  ngf::graphics_pipeline offscreen_pipeline;

  const ngf_attachment_description offscreen_color_attachment_description {
      .type         = NGF_ATTACHMENT_COLOR,
      .format       = NGF_IMAGE_FORMAT_BGRA8_SRGB,
      .sample_count = NGF_SAMPLE_COUNT_1,
      .is_sampled   = true};
  const ngf_attachment_descriptions attachments_list = {
      .descs  = &offscreen_color_attachment_description,
      .ndescs = 1u,
  };
  const ngf_image_ref img_ref = {
      .image        = output_image.get(),
      .mip_level    = 0u,
      .layer        = 0u,
      .cubemap_face = NGF_CUBEMAP_FACE_COUNT};
  ngf_render_target_info rt_info {&attachments_list, &img_ref};

  offscreen_rt.initialize(rt_info);
  const ngf::shader_stage offscreen_vertex_stage =
      ngf_image_comparison::load_shader_stage("small-triangle", "VSMain", NGF_STAGE_VERTEX);
  const ngf::shader_stage offscreen_fragment_stage =
      ngf_image_comparison::load_shader_stage("small-triangle", "PSMain", NGF_STAGE_FRAGMENT);

  ngf_util_graphics_pipeline_data offscreen_pipeline_data;
  ngf_util_create_default_graphics_pipeline_data(&offscreen_pipeline_data);
  ngf_graphics_pipeline_info& offscreen_pipe_info    = offscreen_pipeline_data.pipeline_info;
  offscreen_pipe_info.nshader_stages                 = 2u;
  offscreen_pipe_info.shader_stages[0]               = offscreen_vertex_stage.get();
  offscreen_pipe_info.shader_stages[1]               = offscreen_fragment_stage.get();
  offscreen_pipe_info.compatible_rt_attachment_descs = &attachments_list;
  offscreen_pipeline.initialize(offscreen_pipe_info);

  // Start drawing to output_image
  ngf_irect2d         offsc_viewport {0, 0, 512, 512};
  ngf_cmd_buffer      offscr_cmd_buf = nullptr;
  ngf_cmd_buffer_info cmd_info       = {};
  ngf_create_cmd_buffer(&cmd_info, &offscr_cmd_buf);
  ngf_start_cmd_buffer(offscr_cmd_buf, frame_token);
  {
    ngf::render_encoder renc {offscr_cmd_buf, offscreen_rt, .0f, 0.0f, 0.0f, 0.0f, 1.0, 0u};
    ngf_cmd_bind_gfx_pipeline(renc, offscreen_pipeline);
    ngf_cmd_viewport(renc, &offsc_viewport);
    ngf_cmd_scissor(renc, &offsc_viewport);
    ngf_cmd_draw(renc, false, 0u, 3u, 1u);
  }
  ngf_submit_cmd_buffers(1, &offscr_cmd_buf);
  ngf_destroy_cmd_buffer(offscr_cmd_buf);
}
