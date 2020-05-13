document.body.onload = addElement;

function addElement () 
{ 
    var wrapperWidth = parseInt(window.getComputedStyle(document.querySelector('#data')).getPropertyValue('width'));
    var tableWidth = parseInt(window.getComputedStyle(document.querySelector('#data table')).getPropertyValue('width'));
    
    if (tableWidth > wrapperWidth) {
        var div = document.createElement('div');
        div.className = 'overflow_indicator';
        document.getElementById('data').insertBefore(div, document.querySelector('.overflow_content'));
    }
}

function filter (obj, filterclass) 
{
    obj.classList.toggle('active');
    
    var data = document.getElementById('data');
    
    if (!data.classList.contains('filter_applied'))
			data.classList.add('filter_applied');
			
    var e = document.querySelectorAll('table tbody tr');
    
    for (var i = 0; i < e.length; i++) {
			if (e[i].classList.contains(filterclass))
				e[i].classList.toggle('show');
    }
		
    var filter_active = document.querySelectorAll('#filter .active');
    
    if (filter_active.length == 0)
			remove_filter();
}

function remove_filter() 
{
	var e = document.querySelectorAll('#filter span');
    
  for (var i = 0; i < e.length; i++)
		e[i].classList.remove('active');

	document.getElementById('data').classList.remove('filter_applied');
	
	var e = document.querySelectorAll('table tbody tr');
	
	for (var i = 0; i < e.length; i++)
		e[i].classList.remove('show');
}

function toggle_table_width(obj) 
{
  obj.classList.toggle('active');
  var e = document.querySelector('#data table');
  e.classList.toggle('full');
}
