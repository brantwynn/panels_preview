(function ($) {

 Drupal.behaviors.panelsPreviewToggle = {
   attach: function (context, settings) {
     $('.modal-content select.widget-preview-tabs').on('change', function() {
       switch(this.value) {
         case 'split':
           $('.modal-content .panels-preview-ctools-form div').children('fieldset').show();
           $('.modal-content .widget-preview-single').show().removeClass('panels-preview-full');
         break;
         case 'edit':
           $('.modal-content .panels-preview-ctools-form div').children('fieldset').show();
           $('.modal-content .widget-preview-single').hide();
         break;
         case 'full':
           $('.modal-content .panels-preview-ctools-form div').children('fieldset').hide();
           $('.modal-content .widget-preview-single').show().addClass('panels-preview-full');
         break;
       }
     });
   }
 }

})(jQuery);
